import { useState } from "react";
import { Link } from "react-router";
import {
  Menu,
  X,
  Github,
  ChevronDown,
} from "lucide-react";
import { PythonIcon, NodejsIcon } from "~/components/SdkIcons";
import { GitHubStats } from "~/components/GitHubStats";

export function Navbar() {
  const [mobileOpen, setMobileOpen] = useState(false);

  return (
    <nav className="fixed w-full z-50 glass">
      <div className="max-w-7xl mx-auto px-4 sm:px-6 lg:px-8">
        <div className="flex items-center justify-between h-16">
          {/* Logo */}
          <Link
            to="/"
            className="flex items-center gap-2 group"
          >
            <img
              src="/logo.png"
              alt="Crous"
              className="w-8 h-8 rounded-lg transition-transform duration-300 group-hover:scale-105 group-hover:rotate-3"
            />
            <span className="text-white text-2xl font-bold tracking-tighter">
              crous
            </span>
          </Link>

          {/* Desktop Navigation */}
          <div className="hidden md:flex items-center gap-1">
            <Link
              to="/docs"
              className="px-4 py-2 text-sm font-medium text-gray-300 hover:text-white transition-colors rounded-md hover:bg-white/5"
            >
              Documentation
            </Link>
            <div className="relative group">
              <button className="flex items-center gap-1 px-4 py-2 text-sm font-medium text-gray-300 hover:text-white transition-colors rounded-md hover:bg-white/5">
                SDKs
                <ChevronDown className="w-3.5 h-3.5 transition-transform duration-300 group-hover:rotate-180" />
              </button>
              <div className="invisible group-hover:visible opacity-0 group-hover:opacity-100 translate-y-2 group-hover:translate-y-0 transition-all duration-300 ease-out absolute top-full left-0 mt-1 w-56 bg-[#09090b]/95 backdrop-blur-2xl border border-tubox-border/50 rounded-xl shadow-[0_10px_50px_rgba(0,0,0,0.5)] p-2">
                <Link
                  to="/docs/python"
                  className="flex items-center gap-3 px-3 py-2.5 rounded-lg text-sm text-gray-300 hover:text-white hover:bg-white/5 transition-colors"
                >
                  <PythonIcon className="w-8 h-8" />
                  <div>
                    <div className="font-medium">Python</div>
                    <div className="text-[11px] text-gray-500">v1.0.4</div>
                  </div>
                </Link>
                <Link
                  to="/docs/nodejs"
                  className="flex items-center gap-3 px-3 py-2.5 rounded-lg text-sm text-gray-300 hover:text-white hover:bg-white/5 transition-colors"
                >
                  <NodejsIcon className="w-8 h-8" />
                  <div>
                    <div className="font-medium">Node.js</div>
                    <div className="text-[11px] text-gray-500">v1.0.4</div>
                  </div>
                </Link>
              </div>
            </div>
            <a
              href="https://github.com/axiomchronicles/crous"
              target="_blank"
              rel="noreferrer"
              className="flex items-center gap-1.5 px-4 py-2 text-sm font-medium text-gray-300 hover:text-white transition-colors rounded-md hover:bg-white/5"
            >
              <Github className="w-4 h-4" />
              GitHub
              <GitHubStats />
            </a>
          </div>

          {/* CTA + Mobile Toggle */}
          <div className="flex items-center gap-3">
            <Link
              to="/docs"
              className="hidden sm:inline-flex bg-white text-black hover:bg-crous-400 hover:shadow-[0_0_20px_rgba(74,222,128,0.4)] px-4 py-2 rounded-md text-sm font-medium transition-all duration-300"
            >
              Get Started
            </Link>
            <button
              onClick={() => setMobileOpen(!mobileOpen)}
              className="md:hidden p-2 rounded-md text-gray-400 hover:text-white hover:bg-gray-700 transition-colors"
              aria-label="Toggle menu"
            >
              {mobileOpen ? (
                <X className="w-5 h-5" />
              ) : (
                <Menu className="w-5 h-5" />
              )}
            </button>
          </div>
        </div>
      </div>

      {/* Mobile Menu */}
      {mobileOpen && (
        <div className="md:hidden border-t border-tubox-border/50 bg-[#09090b]/95 backdrop-blur-2xl">
          <div className="px-4 py-4 space-y-1">
            <Link
              to="/docs"
              onClick={() => setMobileOpen(false)}
              className="block px-3 py-2 rounded-lg text-base text-gray-300 hover:text-white hover:bg-white/5"
            >
              Documentation
            </Link>
            <Link
              to="/docs/python"
              onClick={() => setMobileOpen(false)}
              className="block px-3 py-2 rounded-lg text-base text-gray-300 hover:text-white hover:bg-white/5"
            >
              Python SDK
            </Link>
            <Link
              to="/docs/nodejs"
              onClick={() => setMobileOpen(false)}
              className="block px-3 py-2 rounded-lg text-base text-gray-300 hover:text-white hover:bg-white/5"
            >
              Node.js SDK
            </Link>
            <a
              href="https://github.com/axiomchronicles/crous"
              target="_blank"
              rel="noreferrer"
              className="flex items-center gap-2 px-3 py-2 rounded-lg text-base text-gray-300 hover:text-white hover:bg-white/5"
            >
              <Github className="w-4 h-4" />
              GitHub
            </a>
            <Link
              to="/docs"
              onClick={() => setMobileOpen(false)}
              className="block mt-3 bg-white text-black text-center px-4 py-2 rounded-md text-sm font-medium"
            >
              Get Started
            </Link>
          </div>
        </div>
      )}
    </nav>
  );
}
