@echo off
REM Deploy the rngtests web app to VPS-B (prng-nist-tests.student-dev.ro, studlab stack).
REM Mirrors the king-shell / workbench deploy pattern (cwRsync + ssh + compose).
REM
REM Run from rngtests\web\ :  deploy_rngtests.bat        (web-only; skips docs regen)
REM                           deploy_rngtests.bat docs   (also regenerates figures + referat)
REM One-time on the server (before the first deploy):
REM   docker exec studlab-certbot certbot certonly --webroot -w /var/www/certbot -d prng-nist-tests.student-dev.ro

setlocal
set SSH_KEY=%USERPROFILE%\.ssh\student_dev_key
set CWRSYNCHOME=C:\ProgramData\chocolatey\lib\rsync\tools
set "PATH=%CWRSYNCHOME%\bin;%PATH%"
REM One shared set of SSH options for every hop (ssh, scp, rsync) so host-key
REM handling is identical and non-interactive: StrictHostKeyChecking=accept-new
REM trusts a new host key without a prompt, and an explicit known_hosts avoids
REM HOME-resolution surprises. Without this, bare ssh/scp fall back to the
REM interactive "ask" policy and die with "Host key verification failed".
set SSH_OPTS=-i %SSH_KEY% -o UserKnownHostsFile=%USERPROFILE%\.ssh\known_hosts -o StrictHostKeyChecking=accept-new
set RSYNC_RSH=ssh %SSH_OPTS%
set SERVER=admin@student-dev.ro
set REMOTE=/home/admin/data/rngtests

REM The figures (1e6-trial Monte-Carlo) + the XeLaTeX referat are slow and rarely
REM change during web iteration, so skip them by default. Pass "docs" to force a
REM regen; also auto-regen if the referat PDF is missing (e.g. a fresh checkout).
set DO_DOCS=
if /I "%~1"=="docs" set DO_DOCS=1
if not exist "..\docs\main.pdf" set DO_DOCS=1

REM --- 1. Regenerate local artifacts the image needs (data-static + public) ---
pushd ..
echo [1/5] building C++ (for local data generation)...
cmake --build build || goto :fail
if not defined DO_DOCS goto :skipdocs
echo [2/5] regenerating figures + referat...
python docs\make_figures.py || goto :fail
pushd docs && latexmk -interaction=nonstopmode main.tex && popd
goto :afterdocs
:skipdocs
echo [2/5] skipping figures + referat regen (pass "docs" to force)...
:afterdocs
popd
echo [3/5] generating web data-static + public assets...
call pnpm gen || goto :fail

REM --- 2. Push the project (context = rngtests root) ---
echo [4/5] rsync to %SERVER%:%REMOTE% ...
pushd ..
ssh -n %SSH_OPTS% %SERVER% "mkdir -p %REMOTE%"
rsync -rlptD --delete --chmod=D755,F644 --info=stats1 ^
  --exclude=web/node_modules --exclude=web/.next --exclude=build ^
  --exclude=reference --exclude=docs --exclude=*.mp4 --exclude=*.zip ^
  ./ %SERVER%:%REMOTE%/ || (popd & goto :fail)
popd
scp %SSH_OPTS% nginx\prng-nist-tests.conf %SERVER%:/home/admin/data/studlab/nginx/conf.d/prng-nist-tests.conf

REM --- 3. Build + start the container, reload nginx ---
echo [5/5] docker compose up + nginx reload ...
REM No global "docker builder prune" here: VPS-B is a shared host (~30 student
REM projects) and pruning all build cache would evict their cached layers.
ssh -n %SSH_OPTS% %SERVER% "cd %REMOTE%/web && docker compose up -d --build"
ssh -n %SSH_OPTS% %SERVER% "docker exec studlab-nginx nginx -t && docker exec studlab-nginx nginx -s reload"

echo Done. https://prng-nist-tests.student-dev.ro
goto :eof

:fail
echo DEPLOY FAILED.
exit /b 1
