import type { NextConfig } from "next";
import createMDX from "@next/mdx";
import remarkGfm from "remark-gfm";
import remarkMath from "remark-math";
import rehypeKatex from "rehype-katex";
import rehypePrettyCode from "rehype-pretty-code";

/**
 * MDX pipeline: remark-gfm enables GitHub-flavored markdown (tables, etc.),
 * remark-math + rehype-katex render LaTeX math, rehype-pretty-code (Shiki)
 * highlights fenced code blocks at build time.
 */
const withMDX = createMDX({
  extension: /\.mdx?$/,
  options: {
    remarkPlugins: [remarkGfm, remarkMath],
    rehypePlugins: [
      [rehypeKatex, {}],
      [rehypePrettyCode, { theme: "github-dark", keepBackground: false }],
    ],
  },
});

const nextConfig: NextConfig = {
  // Self-contained server bundle for the Docker deploy.
  output: "standalone",
  // Allow .md/.mdx files to be routes/pages.
  pageExtensions: ["ts", "tsx", "md", "mdx"],
};

export default withMDX(nextConfig);
