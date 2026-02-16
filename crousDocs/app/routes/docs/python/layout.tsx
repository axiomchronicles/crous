import { Outlet, Link, useLocation } from "react-router";
import {
  BookOpen,
  Download,
  Database,
  Box,
  Radio,
  Puzzle,
  FileText,
  AlertCircle,
  Code,
  ArrowLeft,
  ArrowRight,
} from "lucide-react";
import { PythonIcon } from "~/components/SdkIcons";

const sidebarSections = [
  {
    title: "Getting Started",
    items: [
      { label: "Introduction", to: "/docs/python", icon: BookOpen, page: "index" },
      { label: "Installation", to: "/docs/python/getting-started", icon: Download, page: "getting-started" },
    ],
  },
  {
    title: "Core",
    items: [
      { label: "Serialization", to: "/docs/python/serialization", icon: Database, page: "serialization" },
      { label: "Type System", to: "/docs/python/types", icon: Box, page: "types" },
      { label: "Streaming", to: "/docs/python/streaming", icon: Radio, page: "streaming" },
      { label: "Custom Types", to: "/docs/python/custom-types", icon: Puzzle, page: "custom-types" },
      { label: "CROUT Format", to: "/docs/python/crout-format", icon: FileText, page: "crout-format" },
    ],
  },
  {
    title: "Reference",
    items: [
      { label: "Error Handling", to: "/docs/python/error-handling", icon: AlertCircle, page: "error-handling" },
      { label: "API Reference", to: "/docs/python/api-reference", icon: Code, page: "api-reference" },
    ],
  },
];

const allPages = sidebarSections.flatMap((s) => s.items);

export default function PythonDocsLayout() {
  const location = useLocation();
  const currentPath = location.pathname;

  const currentIndex = allPages.findIndex((p) => p.to === currentPath);
  const prevPage = currentIndex > 0 ? allPages[currentIndex - 1] : null;
  const nextPage =
    currentIndex < allPages.length - 1 ? allPages[currentIndex + 1] : null;

  return (
    <div className="relative">
      {/* Ambient backgrounds */}
      <div className="fixed inset-0 pointer-events-none">
        <div className="absolute top-0 left-0 w-96 h-96 bg-crous-500/5 rounded-full blur-3xl" />
        <div className="absolute bottom-0 right-0 w-96 h-96 bg-blue-500/5 rounded-full blur-3xl" />
      </div>

      <div className="flex flex-col lg:flex-row max-w-[90rem] mx-auto relative">
        {/* Sidebar */}
        <aside className="hidden lg:block lg:w-72 flex-shrink-0 sticky top-16 h-[calc(100vh-4rem)] overflow-y-auto scrollbar-thin pt-8 pb-12 px-6 border-r border-white/5">
          {/* Back to SDKs */}
          <Link
            to="/docs"
            className="flex items-center gap-2 text-sm text-gray-400 hover:text-white mb-6 transition-colors"
          >
            <ArrowLeft className="w-3.5 h-3.5" />
            Back to SDKs
          </Link>

          {/* SDK Badge */}
          <div className="relative mb-8 group">
            <div className="absolute -inset-0.5 bg-gradient-to-r from-crous-500 to-blue-500 rounded-xl blur opacity-20 group-hover:opacity-40 transition-opacity" />
            <div className="relative flex items-center gap-3 p-3 bg-tubox-card border border-tubox-border rounded-xl">
              <PythonIcon className="w-10 h-10" />
              <div>
                <div className="text-white text-sm font-bold">Python SDK</div>
                <span className="text-xs font-mono text-crous-400">v2.0.0</span>
              </div>
            </div>
          </div>

          {/* Navigation */}
          <nav className="space-y-6">
            {sidebarSections.map((section) => (
              <div key={section.title}>
                <h3 className="text-xs font-bold text-gray-500 uppercase tracking-widest mb-3 px-3">
                  {section.title}
                </h3>
                <ul className="space-y-1">
                  {section.items.map((item) => {
                    const isActive = currentPath === item.to;
                    const Icon = item.icon;
                    return (
                      <li key={item.to}>
                        <Link
                          to={item.to}
                          className={`flex items-center gap-3 px-3 py-2 rounded-lg text-sm transition-all ${
                            isActive
                              ? "bg-gradient-to-r from-crous-500/10 to-transparent text-crous-400 font-medium border-l-2 border-crous-500"
                              : "text-gray-400 hover:text-white hover:bg-white/5"
                          }`}
                        >
                          <Icon className="w-4 h-4 flex-shrink-0" />
                          {item.label}
                        </Link>
                      </li>
                    );
                  })}
                </ul>
              </div>
            ))}
          </nav>
        </aside>

        {/* Content */}
        <main className="flex-1 min-w-0 px-4 sm:px-6 lg:px-12 py-12 max-w-4xl">
          <div className="prose-docs">
            <Outlet />
          </div>

          {/* Prev/Next Navigation */}
          <div className="pt-12 mt-12 border-t border-white/10 flex items-center justify-between">
            {prevPage ? (
              <Link
                to={prevPage.to}
                className="group flex items-center gap-2 text-gray-400 hover:text-white transition-colors"
              >
                <ArrowLeft className="w-4 h-4 group-hover:-translate-x-1 transition-transform" />
                <span className="text-sm">{prevPage.label}</span>
              </Link>
            ) : (
              <div />
            )}
            {nextPage ? (
              <Link
                to={nextPage.to}
                className="group flex items-center gap-2 text-white font-bold hover:text-crous-400 transition-colors"
              >
                <span className="text-sm">{nextPage.label}</span>
                <ArrowRight className="w-4 h-4 group-hover:translate-x-1 transition-transform" />
              </Link>
            ) : (
              <div />
            )}
          </div>
        </main>
      </div>
    </div>
  );
}
