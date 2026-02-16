import { Link } from "react-router";
import { Github } from "lucide-react";

export function Footer() {
  return (
    <footer className="border-t border-tubox-border bg-tubox-card/50 backdrop-blur-sm mt-auto">
      <div className="max-w-7xl mx-auto py-12 px-4 sm:px-6 lg:px-8">
        <div className="flex flex-col sm:flex-row justify-between items-center gap-4">
          <div className="flex items-center gap-3">
            <img
              src="/logo.png"
              alt="Crous"
              className="w-6 h-6 rounded opacity-80"
            />
            <span className="text-gray-500 text-sm">
              © {new Date().getFullYear()} Crous. MIT License.
            </span>
          </div>
          <div className="flex items-center gap-4">
            <a
              href="https://github.com/axiomchronicles/crous"
              target="_blank"
              rel="noreferrer"
              className="text-gray-400 hover:text-crous-400 transition-colors"
            >
              <Github className="w-5 h-5" />
            </a>
          </div>
        </div>
      </div>
    </footer>
  );
}
