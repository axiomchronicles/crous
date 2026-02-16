import {
  Links,
  Meta,
  Outlet,
  Scripts,
  ScrollRestoration,
} from "react-router";
import type { Route } from "./+types/root";
import "./app.css";

export const links: Route.LinksFunction = () => [
  { rel: "preconnect", href: "https://fonts.googleapis.com" },
  {
    rel: "preconnect",
    href: "https://fonts.gstatic.com",
    crossOrigin: "anonymous",
  },
  {
    rel: "stylesheet",
    href: "https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;500;600;700;800&family=Space+Mono:wght@400;700&display=swap",
  },
  // SEO & PWA
  { rel: "canonical", href: "https://crous.dev" },
  { rel: "manifest", href: "/manifest.json" },
  { rel: "sitemap", type: "application/xml", href: "/sitemap.xml" },
  // Favicons
  { rel: "icon", type: "image/x-icon", href: "/favicon.ico" },
  { rel: "icon", type: "image/png", sizes: "512x512", href: "/logo.png" },
  { rel: "apple-touch-icon", href: "/logo.png" },
];

export function Layout({ children }: { children: React.ReactNode }) {
  return (
    <html lang="en" className="dark">
      <head>
        <meta charSet="utf-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=5" />
        
        {/* Primary Meta Tags */}
        <meta name="title" content="Crous - Deterministic Binary Document Format" />
        <meta
          name="description"
          content="High-performance binary serialization library for Python and Node.js. Smaller than JSON, no schema required. C core with deterministic output for hashing and caching."
        />
        <meta name="keywords" content="binary serialization, data format, python library, nodejs library, binary encoding, msgpack alternative, protobuf alternative, json alternative, deterministic serialization, high performance" />
        <meta name="author" content="Crous Team" />
        <meta name="robots" content="index, follow" />
        <meta name="language" content="English" />
        <meta name="revisit-after" content="7 days" />
        
        {/* Open Graph / Facebook */}
        <meta property="og:type" content="website" />
        <meta property="og:url" content="https://crous.dev/" />
        <meta property="og:title" content="Crous - Deterministic Binary Document Format" />
        <meta property="og:description" content="High-performance binary serialization library for Python and Node.js. Smaller than JSON, no schema required." />
        <meta property="og:image" content="https://crous.dev/logo.png" />
        <meta property="og:site_name" content="Crous" />
        <meta property="og:locale" content="en_US" />
        
        {/* Twitter */}
        <meta name="twitter:card" content="summary_large_image" />
        <meta name="twitter:url" content="https://crous.dev/" />
        <meta name="twitter:title" content="Crous - Deterministic Binary Document Format" />
        <meta name="twitter:description" content="High-performance binary serialization library for Python and Node.js. Smaller than JSON, no schema required." />
        <meta name="twitter:image" content="https://crous.dev/logo.png" />
        <meta name="twitter:creator" content="@axiomchronicles" />
        
        {/* Additional Meta */}
        <meta name="theme-color" content="#22c55e" />
        <meta name="color-scheme" content="dark" />
        <meta name="format-detection" content="telephone=no" />
        
        {/* Structured Data - Organization */}
        <script type="application/ld+json">
          {JSON.stringify({
            "@context": "https://schema.org",
            "@type": "SoftwareApplication",
            "name": "Crous",
            "applicationCategory": "DeveloperApplication",
            "operatingSystem": "Cross-platform",
            "description": "High-performance binary serialization library for Python and Node.js. Deterministic output, no schema required.",
            "offers": {
              "@type": "Offer",
              "price": "0",
              "priceCurrency": "USD"
            },
            "programmingLanguage": ["Python", "JavaScript", "C"],
            "author": {
              "@type": "Organization",
              "name": "Crous Team",
              "url": "https://github.com/axiomchronicles/crous"
            },
            "url": "https://crous.dev",
            "sameAs": [
              "https://github.com/axiomchronicles/crous",
              "https://pypi.org/project/crous/",
              "https://www.npmjs.com/package/crous"
            ]
          })}
        </script>
        
        <Meta />
        <Links />
      </head>
      <body className="min-h-screen flex flex-col">
        {children}
        <ScrollRestoration />
        <Scripts />
      </body>
    </html>
  );
}

export default function App() {
  return <Outlet />;
}

export function ErrorBoundary({ error }: Route.ErrorBoundaryProps) {
  return (
    <div className="min-h-screen bg-[#02040a] flex items-center justify-center">
      <div className="text-center">
        <h1 className="text-6xl font-bold text-white mb-4">404</h1>
        <p className="text-gray-400 text-lg">Page not found</p>
      </div>
    </div>
  );
}
