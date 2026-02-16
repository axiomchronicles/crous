import { createRequestHandler } from "react-router";

export const handler = createRequestHandler({
  // @ts-ignore
  build: () => import("../../build/server/index.js"),
});
