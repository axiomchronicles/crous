import type { Config } from "@netlify/functions";

export default async (request: Request) => {
  const { createRequestHandler } = await import("react-router");
  const build = await import("../../build/server/index.js");
  
  const handler = createRequestHandler({
    build,
    mode: process.env.NODE_ENV as "development" | "production",
  });
  
  return handler(request);
};

export const config: Config = {
  path: "/*",
  preferStatic: true,
};
