import type { Context } from "@netlify/functions";

export default async (request: Request, context: Context) => {
  const { createRequestHandler } = await import("react-router");
  // @ts-ignore
  const build = await import("../../build/server/index.js");
  
  const handler = createRequestHandler({ build });
  return handler(request, context);
};
