import { useState } from "react";
import { Copy, Check } from "lucide-react";

interface CodeBlockProps {
  code: string;
  language?: string;
  filename?: string;
  showLineNumbers?: boolean;
}

/* ─── Lightweight custom syntax highlighter ───────────────────────── */

const KEYWORDS: Record<string, Set<string>> = {
  python: new Set([
    "import", "from", "def", "class", "return", "if", "elif", "else",
    "for", "while", "with", "as", "try", "except", "finally", "raise",
    "yield", "lambda", "pass", "break", "continue", "and", "or", "not",
    "in", "is", "del", "global", "nonlocal", "async", "await", "assert",
  ]),
  javascript: new Set([
    "import", "from", "export", "default", "function", "const", "let",
    "var", "return", "if", "else", "for", "while", "do", "switch", "case",
    "break", "continue", "try", "catch", "finally", "throw", "new",
    "class", "extends", "super", "this", "typeof", "instanceof", "of",
    "in", "async", "await", "yield", "delete", "void",
  ]),
  typescript: new Set([
    "import", "from", "export", "default", "function", "const", "let",
    "var", "return", "if", "else", "for", "while", "do", "switch", "case",
    "break", "continue", "try", "catch", "finally", "throw", "new",
    "class", "extends", "super", "this", "typeof", "instanceof", "of",
    "in", "async", "await", "yield", "delete", "void",
    "type", "interface", "enum", "implements", "declare", "as",
    "readonly", "abstract", "private", "protected", "public",
  ]),
  c: new Set([
    "int", "char", "float", "double", "void", "long", "short", "unsigned",
    "signed", "struct", "union", "enum", "typedef", "const", "static",
    "extern", "return", "if", "else", "for", "while", "do", "switch",
    "case", "break", "continue", "sizeof", "NULL", "goto", "default",
    "volatile", "register", "inline", "restrict",
    "int8_t", "int16_t", "int32_t", "int64_t", "uint8_t", "uint16_t",
    "uint32_t", "uint64_t", "size_t", "bool",
  ]),
  bash: new Set([
    "if", "then", "else", "elif", "fi", "for", "while", "do", "done",
    "case", "esac", "in", "function", "return", "export", "source",
    "local", "echo", "exit", "cd", "ls", "grep", "sed", "awk",
    "pip", "npm", "git", "python", "node", "curl", "mkdir",
  ]),
};

const BUILTINS: Record<string, Set<string>> = {
  python: new Set([
    "True", "False", "None", "print", "len", "range", "type", "int",
    "float", "str", "list", "dict", "set", "tuple", "bool", "bytes",
    "isinstance", "hasattr", "getattr", "setattr", "open", "super",
    "property", "staticmethod", "classmethod", "enumerate", "zip", "map",
    "filter", "sorted", "reversed", "any", "all", "min", "max", "sum",
    "abs", "round", "hex", "oct", "bin", "id", "hash", "repr",
    "Exception", "TypeError", "ValueError", "KeyError", "IndexError",
    "RuntimeError", "StopIteration", "NotImplementedError", "AttributeError",
  ]),
  javascript: new Set([
    "true", "false", "null", "undefined", "NaN", "Infinity",
    "console", "Math", "Date", "JSON", "Object", "Array", "Map", "Set",
    "Buffer", "Error", "TypeError", "Promise", "Symbol", "Proxy",
    "parseInt", "parseFloat", "isNaN", "require", "module", "exports",
    "process", "setTimeout", "setInterval", "clearTimeout",
  ]),
  typescript: new Set([
    "true", "false", "null", "undefined", "NaN", "Infinity",
    "console", "Math", "Date", "JSON", "Object", "Array", "Map", "Set",
    "Buffer", "Error", "TypeError", "Promise", "Symbol", "Proxy",
    "parseInt", "parseFloat", "isNaN", "require", "module", "exports",
    "string", "number", "boolean", "any", "never", "unknown", "void",
    "Record", "Partial", "Readonly", "Pick", "Omit",
  ]),
  c: new Set([
    "true", "false", "NULL", "stdin", "stdout", "stderr",
    "printf", "fprintf", "sprintf", "scanf", "malloc", "calloc",
    "realloc", "free", "memcpy", "memset", "strlen", "strcmp",
    "strncpy", "fopen", "fclose", "fread", "fwrite",
  ]),
  bash: new Set([]),
};

interface Token {
  text: string;
  type: "keyword" | "string" | "number" | "comment" | "builtin" | "function" | "operator" | "punctuation" | "plain" | "decorator";
}

function tokenize(code: string, lang: string): Token[] {
  const keywords = KEYWORDS[lang] || KEYWORDS.bash || new Set();
  const builtins = BUILTINS[lang] || new Set();
  const tokens: Token[] = [];
  let i = 0;

  while (i < code.length) {
    // Comments
    if ((lang === "python" || lang === "bash") && code[i] === "#") {
      let end = code.indexOf("\n", i);
      if (end === -1) end = code.length;
      tokens.push({ text: code.slice(i, end), type: "comment" });
      i = end;
      continue;
    }
    if ((lang === "javascript" || lang === "typescript" || lang === "c") && code[i] === "/" && code[i + 1] === "/") {
      let end = code.indexOf("\n", i);
      if (end === -1) end = code.length;
      tokens.push({ text: code.slice(i, end), type: "comment" });
      i = end;
      continue;
    }
    if ((lang === "javascript" || lang === "typescript" || lang === "c") && code[i] === "/" && code[i + 1] === "*") {
      let end = code.indexOf("*/", i + 2);
      if (end === -1) end = code.length; else end += 2;
      tokens.push({ text: code.slice(i, end), type: "comment" });
      i = end;
      continue;
    }

    // Decorators (Python)
    if (lang === "python" && code[i] === "@" && (i === 0 || code[i-1] === "\n" || /\s/.test(code[i-1]))) {
      let end = i + 1;
      while (end < code.length && /[\w.]/.test(code[end])) end++;
      tokens.push({ text: code.slice(i, end), type: "decorator" });
      i = end;
      continue;
    }

    // Strings
    if (code[i] === '"' || code[i] === "'") {
      const quote = code[i];
      // Triple quotes
      if (code.slice(i, i + 3) === quote.repeat(3)) {
        let end = code.indexOf(quote.repeat(3), i + 3);
        if (end === -1) end = code.length; else end += 3;
        tokens.push({ text: code.slice(i, end), type: "string" });
        i = end;
        continue;
      }
      // Single/double quoted
      let end = i + 1;
      while (end < code.length && code[end] !== quote && code[end] !== "\n") {
        if (code[end] === "\\") end++;
        end++;
      }
      if (end < code.length) end++;
      tokens.push({ text: code.slice(i, end), type: "string" });
      i = end;
      continue;
    }

    // Template literals
    if (code[i] === "`") {
      let end = i + 1;
      let depth = 0;
      while (end < code.length) {
        if (code[end] === "\\" && end + 1 < code.length) { end += 2; continue; }
        if (code[end] === "$" && code[end+1] === "{") { depth++; end += 2; continue; }
        if (code[end] === "}" && depth > 0) { depth--; end++; continue; }
        if (code[end] === "`" && depth === 0) { end++; break; }
        end++;
      }
      tokens.push({ text: code.slice(i, end), type: "string" });
      i = end;
      continue;
    }

    // Numbers
    if (/\d/.test(code[i]) || (code[i] === "." && i + 1 < code.length && /\d/.test(code[i+1]))) {
      let end = i;
      if (code[end] === "0" && (code[end+1] === "x" || code[end+1] === "X" || code[end+1] === "b" || code[end+1] === "o")) {
        end += 2;
        while (end < code.length && /[\da-fA-F_]/.test(code[end])) end++;
      } else {
        while (end < code.length && /[\d._eE+\-]/.test(code[end])) end++;
      }
      tokens.push({ text: code.slice(i, end), type: "number" });
      i = end;
      continue;
    }

    // Words (identifiers, keywords)
    if (/[\w$]/.test(code[i])) {
      let end = i;
      while (end < code.length && /[\w$]/.test(code[end])) end++;
      const word = code.slice(i, end);
      if (keywords.has(word)) {
        tokens.push({ text: word, type: "keyword" });
      } else if (builtins.has(word)) {
        tokens.push({ text: word, type: "builtin" });
      } else if (end < code.length && code[end] === "(") {
        tokens.push({ text: word, type: "function" });
      } else {
        tokens.push({ text: word, type: "plain" });
      }
      i = end;
      continue;
    }

    // Operators / punctuation
    if (/[+\-*/%=<>!&|^~?:]/.test(code[i])) {
      let end = i + 1;
      while (end < code.length && /[+\-*/%=<>!&|^~?:]/.test(code[end])) end++;
      tokens.push({ text: code.slice(i, end), type: "operator" });
      i = end;
      continue;
    }

    if (/[{}()\[\],;.]/.test(code[i])) {
      tokens.push({ text: code[i], type: "punctuation" });
      i++;
      continue;
    }

    // Whitespace / newlines
    if (/\s/.test(code[i])) {
      let end = i;
      while (end < code.length && /\s/.test(code[end]) && code[end] !== "\n") end++;
      if (code[i] === "\n") {
        tokens.push({ text: "\n", type: "plain" });
        i++;
      } else {
        tokens.push({ text: code.slice(i, end), type: "plain" });
        i = end;
      }
      continue;
    }

    // Fallback
    tokens.push({ text: code[i], type: "plain" });
    i++;
  }

  return tokens;
}

const COLOR_MAP: Record<Token["type"], string> = {
  keyword: "text-purple-400",
  string: "text-green-400",
  number: "text-blue-400",
  comment: "text-gray-500",
  builtin: "text-crous-400",
  function: "text-yellow-300",
  operator: "text-gray-400",
  punctuation: "text-gray-400",
  plain: "text-gray-300",
  decorator: "text-yellow-300",
};

/* ─── Component ───────────────────────────────────────────────────── */

function getLang(language: string): string {
  const map: Record<string, string> = {
    py: "python", python: "python",
    js: "javascript", javascript: "javascript",
    ts: "typescript", typescript: "typescript",
    c: "c", h: "c", cpp: "c",
    bash: "bash", sh: "bash", shell: "bash", zsh: "bash",
    json: "javascript", jsx: "javascript", tsx: "typescript",
  };
  return map[language.toLowerCase()] || "bash";
}

export function CodeBlock({
  code,
  language = "python",
  filename,
  showLineNumbers = false,
}: CodeBlockProps) {
  const [copied, setCopied] = useState(false);

  const handleCopy = () => {
    navigator.clipboard.writeText(code);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  const lang = getLang(language);
  const tokens = tokenize(code.trim(), lang);

  return (
    <div className="code-window group relative mb-6">
      {filename && (
        <div className="code-window-header">
          <div className="flex gap-1.5">
            <div className="w-2.5 h-2.5 rounded-full bg-[#FF5F56]/50" />
            <div className="w-2.5 h-2.5 rounded-full bg-[#FFBD2E]/50" />
            <div className="w-2.5 h-2.5 rounded-full bg-[#27C93F]/50" />
          </div>
          <span className="text-xs uppercase font-mono text-gray-500 tracking-wider ml-2">
            {filename}
          </span>
          <div className="flex-1" />
          <button
            onClick={handleCopy}
            className="p-1.5 rounded-md text-gray-400 hover:text-white hover:bg-white/10 opacity-0 group-hover:opacity-100 transition-all"
          >
            {copied ? (
              <Check className="w-3.5 h-3.5 text-crous-400" />
            ) : (
              <Copy className="w-3.5 h-3.5" />
            )}
          </button>
        </div>
      )}
      <div className="relative">
        {!filename && (
          <button
            onClick={handleCopy}
            className="absolute top-3 right-3 p-1.5 rounded-md text-gray-400 hover:text-white hover:bg-white/10 opacity-0 group-hover:opacity-100 transition-all z-10"
          >
            {copied ? (
              <Check className="w-3.5 h-3.5 text-crous-400" />
            ) : (
              <Copy className="w-3.5 h-3.5" />
            )}
          </button>
        )}
        <pre className="bg-black/50 p-4 sm:p-6 overflow-x-auto">
          <code className="font-mono text-sm leading-relaxed">
            {showLineNumbers
              ? (() => {
                  const lines: Token[][] = [[]];
                  tokens.forEach((t) => {
                    if (t.text === "\n") {
                      lines.push([]);
                    } else {
                      lines[lines.length - 1].push(t);
                    }
                  });
                  return lines.map((line, i) => (
                    <span key={i} className="block">
                      <span className="inline-block w-8 text-right mr-4 text-gray-600 select-none text-xs">
                        {i + 1}
                      </span>
                      {line.map((tok, j) => (
                        <span key={j} className={COLOR_MAP[tok.type]}>
                          {tok.text}
                        </span>
                      ))}
                    </span>
                  ));
                })()
              : tokens.map((tok, i) => (
                  <span key={i} className={COLOR_MAP[tok.type]}>
                    {tok.text}
                  </span>
                ))}
          </code>
        </pre>
      </div>
    </div>
  );
}
