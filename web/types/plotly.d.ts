// The slim Plotly bundle and the react-plotly.js factory subpath ship no types.
// We use them only behind a client-only dynamic import, so `any` is acceptable.
declare module "plotly.js-dist-min";
declare module "react-plotly.js/factory";
