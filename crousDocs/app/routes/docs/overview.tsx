import { Link } from "react-router";
import { ArrowRight } from "lucide-react";
import { PythonIcon, NodejsIcon } from "~/components/SdkIcons";
import type { Route } from "./+types/overview";

export function meta({}: Route.MetaArgs) {
  return [
    { title: "Documentation - Crous Binary Serialization Library" },
    { 
      name: "description", 
      content: "Official Crous documentation for Python and Node.js. Learn binary serialization, API reference, streaming, custom types, and performance optimization." 
    },
    { 
      name: "keywords", 
      content: "crous documentation, binary serialization docs, python serialization tutorial, nodejs binary encoding, API reference, data format guide" 
    },
    { property: "og:title", content: "Crous Documentation" },
    { property: "og:description", content: "Official documentation for Crous binary serialization library. Python and Node.js guides." },
    { tagName: "link", rel: "canonical", href: "https://crous.dev/docs" },
  ];
}

export default function DocsOverview() {
  return (
    <div className="relative min-h-[calc(100vh-4rem)]">
      {/* Background */}
      <div className="absolute inset-0 bg-[#050505]" />
      <div className="absolute inset-0 bg-grid opacity-10" />
      <div className="absolute top-0 left-1/2 -translate-x-1/2 w-[600px] h-[400px] bg-crous-500/10 rounded-[100%] blur-[100px] opacity-40 pointer-events-none" />

      <div className="relative max-w-7xl mx-auto px-6 lg:px-8 pt-20 pb-32">
        {/* Header */}
        <div className="text-center mb-16">
          <h1 className="text-4xl md:text-5xl font-extrabold tracking-tight mb-4">
            <span className="gradient-text">Documentation</span>
          </h1>
          <p className="text-lg text-gray-400 font-light max-w-xl mx-auto">
            Everything you need to integrate Crous into your applications.
            Choose your platform below.
          </p>
        </div>

        {/* SDK Cards */}
        <div className="grid grid-cols-1 md:grid-cols-2 gap-8 max-w-4xl mx-auto mb-20">
          {/* Python SDK */}
          <Link
            to="/docs/python"
            className="group relative p-8 bg-[#0A0A0A] border border-white/10 rounded-3xl hover:-translate-y-1 hover:shadow-2xl hover:shadow-crous-900/20 transition-all duration-300"
          >
            <div className="absolute -inset-0.5 bg-gradient-to-r from-[#3776AB] to-crous-500 rounded-3xl blur opacity-0 group-hover:opacity-20 transition-opacity duration-300" />
            <div className="relative">
              <div className="flex items-center gap-4 mb-6">
                <PythonIcon className="w-14 h-14" />
                <div>
                  <h2 className="text-2xl font-bold text-white">Python SDK</h2>
                  <span className="text-xs px-2 py-1 bg-crous-500/10 text-crous-400 rounded-full border border-crous-500/20 font-mono">
                    v2.0.0
                  </span>
                </div>
              </div>
              <p className="text-gray-400 text-sm font-light leading-relaxed mb-6">
                C extension for Python with full FLUX binary, CROUT text, and
                streaming support. Compatible with Python 3.6+.
              </p>
              <div className="flex items-center text-crous-400 text-sm font-medium">
                Get Started
                <ArrowRight className="w-4 h-4 ml-1 group-hover:translate-x-1 transition-transform" />
              </div>
            </div>
          </Link>

          {/* Node.js SDK */}
          <Link
            to="/docs/nodejs"
            className="group relative p-8 bg-[#0A0A0A] border border-white/10 rounded-3xl hover:-translate-y-1 hover:shadow-2xl hover:shadow-crous-900/20 transition-all duration-300"
          >
            <div className="absolute -inset-0.5 bg-gradient-to-r from-[#339933] to-crous-500 rounded-3xl blur opacity-0 group-hover:opacity-20 transition-opacity duration-300" />
            <div className="relative">
              <div className="flex items-center gap-4 mb-6">
                <NodejsIcon className="w-14 h-14" />
                <div>
                  <h2 className="text-2xl font-bold text-white">Node.js SDK</h2>
                  <span className="text-xs px-2 py-1 bg-crous-500/10 text-crous-400 rounded-full border border-crous-500/20 font-mono">
                    v2.0.0
                  </span>
                </div>
              </div>
              <p className="text-gray-400 text-sm font-light leading-relaxed mb-6">
                N-API native addon for Node.js with ABI stability. File I/O,
                custom serializers, and cross-platform support.
              </p>
              <div className="flex items-center text-crous-400 text-sm font-medium">
                Get Started
                <ArrowRight className="w-4 h-4 ml-1 group-hover:translate-x-1 transition-transform" />
              </div>
            </div>
          </Link>
        </div>

        {/* Quick Links */}
        <div className="max-w-4xl mx-auto">
          <h3 className="text-lg font-bold text-white mb-6 text-center">
            Quick Links
          </h3>
          <div className="grid grid-cols-1 sm:grid-cols-3 gap-4">
            <QuickLink
              to="/docs/python/api-reference"
              title="Python API Reference"
              description="Complete function & class reference"
            />
            <QuickLink
              to="/docs/nodejs/api-reference"
              title="Node.js API Reference"
              description="Full JavaScript API documentation"
            />
            <QuickLink
              to="/docs/python/types"
              title="Type System"
              description="Supported types and mappings"
            />
          </div>
        </div>
      </div>
    </div>
  );
}

function QuickLink({
  to,
  title,
  description,
}: {
  to: string;
  title: string;
  description: string;
}) {
  return (
    <Link
      to={to}
      className="group p-5 bg-white/[0.02] border border-white/10 rounded-xl hover:border-crous-500/30 hover:bg-white/[0.04] transition-all duration-300"
    >
      <h4 className="text-white font-medium text-sm mb-1 group-hover:text-crous-400 transition-colors">
        {title}
      </h4>
      <p className="text-gray-500 text-xs">{description}</p>
    </Link>
  );
}
