
    export { default } from "../../../build/server/server.js";

    export const config = {
      name: "React Router server handler",
      generator: "@netlify/vite-plugin-react-router@2.1.3",
      path: "/*",
      excludedPath: ["/.netlify/*"],
      preferStatic: true,
    };
    