import type { MDXComponents } from "mdx/types";

/**
 * Global MDX component overrides. Required by @next/mdx in the App Router.
 * Styling is applied via the `.prose-paper` wrapper in the paper layout, so we
 * only need to pass components through here.
 */
export function useMDXComponents(components: MDXComponents): MDXComponents {
  return { ...components };
}
