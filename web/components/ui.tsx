import type { ComponentProps } from "react";

// `btn` marks button-styled links so prose styling (.prose-paper a) skips them.
const BASE =
  "btn inline-flex items-center justify-center gap-2 rounded-lg font-medium no-underline transition-colors " +
  "focus-visible:outline-none focus-visible:ring-2 focus-visible:ring-accent/40 " +
  "disabled:opacity-50 disabled:pointer-events-none whitespace-nowrap";

const VARIANTS = {
  primary: "bg-accent text-accent-fg hover:brightness-110 shadow-[var(--shadow)]",
  secondary: "border border-border bg-surface text-foreground hover:border-accent hover:text-accent",
  ghost: "text-muted hover:text-foreground hover:bg-surface-2",
} as const;

const SIZES = {
  sm: "h-9 px-3 text-sm",
  md: "h-11 px-5 text-sm",
  lg: "h-12 px-7 text-base",
} as const;

type Variant = keyof typeof VARIANTS;
type Size = keyof typeof SIZES;

/** Shared button class string, usable on <button> or <Link>/<a>. */
export function buttonClasses(variant: Variant = "primary", size: Size = "md", extra = ""): string {
  return `${BASE} ${VARIANTS[variant]} ${SIZES[size]} ${extra}`;
}

export function Button({
  variant = "primary",
  size = "md",
  className = "",
  ...props
}: ComponentProps<"button"> & { variant?: Variant; size?: Size }) {
  return <button className={buttonClasses(variant, size, className)} {...props} />;
}

/** Standard form control classes (inputs, selects). */
export const fieldClasses =
  "h-11 rounded-lg border border-border bg-surface px-3 text-sm focus-visible:outline-none " +
  "focus-visible:ring-2 focus-visible:ring-accent/40";
