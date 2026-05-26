"use client";

// Builds the React Plotly component from the slim plotly.js bundle. This module
// is only ever loaded in the browser (via the dynamic import in Plot.tsx) since
// plotly.js touches `window` at import time.
import Plotly from "plotly.js-dist-min";
import createPlotlyComponent from "react-plotly.js/factory";

const Plot = createPlotlyComponent(Plotly);

export default Plot;
