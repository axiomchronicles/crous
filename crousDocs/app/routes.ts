import { type RouteConfig, index, route } from "@react-router/dev/routes";

export default [
  index("routes/home.tsx"),
  route("docs", "routes/docs/layout.tsx", [
    index("routes/docs/overview.tsx"),
    route("python", "routes/docs/python/layout.tsx", [
      index("routes/docs/python/index.tsx"),
      route("getting-started", "routes/docs/python/getting-started.tsx"),
      route("serialization", "routes/docs/python/serialization.tsx"),
      route("types", "routes/docs/python/types.tsx"),
      route("streaming", "routes/docs/python/streaming.tsx"),
      route("custom-types", "routes/docs/python/custom-types.tsx"),
      route("crout-format", "routes/docs/python/crout-format.tsx"),
      route("error-handling", "routes/docs/python/error-handling.tsx"),
      route("api-reference", "routes/docs/python/api-reference.tsx"),
    ]),
    route("nodejs", "routes/docs/nodejs/layout.tsx", [
      index("routes/docs/nodejs/index.tsx"),
      route("getting-started", "routes/docs/nodejs/getting-started.tsx"),
      route("serialization", "routes/docs/nodejs/serialization.tsx"),
      route("types", "routes/docs/nodejs/types.tsx"),
      route("custom-types", "routes/docs/nodejs/custom-types.tsx"),
      route("file-io", "routes/docs/nodejs/file-io.tsx"),
      route("error-handling", "routes/docs/nodejs/error-handling.tsx"),
      route("api-reference", "routes/docs/nodejs/api-reference.tsx"),
    ]),
  ]),
] satisfies RouteConfig;
