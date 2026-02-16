/**
 * ArchitectureSvg.tsx — Clean, borderless architecture visualizations
 *
 * DataFlowAnimation: Orbital encoding pipeline with glowing nodes and curved particle trails
 * ArchitectureDiagram: Layered constellation with soft radial glows and flowing connections
 */

/* ─── Encoding Pipeline ───────────────────────────────────────── */
export function DataFlowAnimation() {
  /* Layout constants — 4 nodes spread across a generous canvas */
  const W = 960, H = 440;
  const CY = 195; // vertical centre for all orbs
  const nodes = [
    { cx: 120, cy: CY, label: "Python Dict", sub: "input data" },
    { cx: 340, cy: CY, label: "Type Inspector", sub: "introspect" },
    { cx: 580, cy: CY, label: "C Core Encoder", sub: "encode" },
    { cx: 830, cy: CY, label: "FLUX Binary", sub: "output" },
  ];
  /* Bézier curves connecting each pair */
  const curves = [
    `M${nodes[0].cx + 56} ${CY} C${nodes[0].cx + 100} ${CY - 50} ${nodes[1].cx - 100} ${CY + 50} ${nodes[1].cx - 56} ${CY}`,
    `M${nodes[1].cx + 52} ${CY} C${nodes[1].cx + 95} ${CY - 45} ${nodes[2].cx - 110} ${CY + 45} ${nodes[2].cx - 62} ${CY}`,
    `M${nodes[2].cx + 62} ${CY} C${nodes[2].cx + 110} ${CY - 48} ${nodes[3].cx - 100} ${CY + 48} ${nodes[3].cx - 56} ${CY}`,
  ];

  return (
    <div className="relative w-full overflow-hidden">
      <style>{`
        /* ── Floating & breathing ── */
        @keyframes dfFloat {
          0%, 100% { transform: translateY(0); }
          50% { transform: translateY(-6px); }
        }
        @keyframes dfBreathe {
          0%, 100% { opacity: 0.18; transform: scale(1); }
          50% { opacity: 0.38; transform: scale(1.06); }
        }
        @keyframes dfBreatheSoft {
          0%, 100% { opacity: 0.08; transform: scale(1); }
          50% { opacity: 0.16; transform: scale(1.04); }
        }
        /* ── Entrance ── */
        @keyframes dfNodeIn {
          0% { opacity: 0; transform: scale(0.7) translateY(20px); }
          100% { opacity: 1; transform: scale(1) translateY(0); }
        }
        /* ── Orbit ring rotation ── */
        @keyframes dfOrbit {
          0% { transform: rotate(0deg); }
          100% { transform: rotate(360deg); }
        }
        @keyframes dfOrbitRev {
          0% { transform: rotate(360deg); }
          100% { transform: rotate(0deg); }
        }
        /* ── Pulse ring expansion ── */
        @keyframes dfPulse {
          0% { r: 46; opacity: 0.25; }
          100% { r: 72; opacity: 0; }
        }
        @keyframes dfPulseLg {
          0% { r: 54; opacity: 0.2; }
          100% { r: 85; opacity: 0; }
        }
        /* ── Core label breath ── */
        @keyframes dfCorePulse {
          0%, 100% { opacity: 0.6; }
          50% { opacity: 1; }
        }
        /* ── Sparkle / data fragments ── */
        @keyframes dfSparkle {
          0%, 100% { opacity: 0; transform: scale(0.5) translateY(0); }
          50% { opacity: 1; transform: scale(1) translateY(-3px); }
        }
        /* ── Hex scroll ── */
        @keyframes dfHexScroll {
          0% { transform: translateY(0); }
          100% { transform: translateY(-60px); }
        }
        /* ── Particle travel along curve ── */
        @keyframes dfTravel {
          0% { offset-distance: 0%; opacity: 0; }
          6% { opacity: 1; }
          94% { opacity: 1; }
          100% { offset-distance: 100%; opacity: 0; }
        }
        /* ── Energy flash along curve ── */
        @keyframes dfEnergy {
          0% { offset-distance: 0%; opacity: 0; }
          3% { opacity: 0.9; }
          50% { opacity: 0.5; }
          97% { opacity: 0.9; }
          100% { offset-distance: 100%; opacity: 0; }
        }
        /* ── Flowing gradient ribbon ── */
        @keyframes dfRibbon {
          0% { stroke-dashoffset: 120; }
          100% { stroke-dashoffset: 0; }
        }

        /* ── Float classes ── */
        .df2-float { animation: dfFloat 5s ease-in-out infinite; }
        .df2-float-d1 { animation-delay: 0.6s; }
        .df2-float-d2 { animation-delay: 1.2s; }
        .df2-float-d3 { animation-delay: 1.8s; }

        /* ── Breathe classes ── */
        .df2-breathe { animation: dfBreathe 5s ease-in-out infinite; }
        .df2-breathe-d1 { animation-delay: 0.8s; }
        .df2-breathe-d2 { animation-delay: 1.6s; }
        .df2-breathe-soft { animation: dfBreatheSoft 6s ease-in-out infinite; }

        /* ── Node entrance ── */
        .df2-node-1 { animation: dfNodeIn 0.8s cubic-bezier(0.16,1,0.3,1) 0.1s both; }
        .df2-node-2 { animation: dfNodeIn 0.8s cubic-bezier(0.16,1,0.3,1) 0.25s both; }
        .df2-node-3 { animation: dfNodeIn 0.8s cubic-bezier(0.16,1,0.3,1) 0.4s both; }
        .df2-node-4 { animation: dfNodeIn 0.8s cubic-bezier(0.16,1,0.3,1) 0.55s both; }

        /* ── Orbits ── */
        .df2-orbit { animation: dfOrbit 20s linear infinite; transform-origin: center; }
        .df2-orbit-rev { animation: dfOrbitRev 25s linear infinite; transform-origin: center; }
        .df2-orbit-slow { animation: dfOrbit 35s linear infinite; transform-origin: center; }

        /* ── Pulse rings ── */
        .df2-pulse { animation: dfPulse 3s ease-out infinite; }
        .df2-pulse-d1 { animation-delay: 1s; }
        .df2-pulse-lg { animation: dfPulseLg 3.5s ease-out infinite; }
        .df2-pulse-lg-d1 { animation-delay: 1.2s; }

        /* ── Core pulse ── */
        .df2-core-pulse { animation: dfCorePulse 2.5s ease-in-out infinite; }

        /* ── Sparkle ── */
        .df2-sparkle { animation: dfSparkle 3.5s ease-in-out infinite; }
        .df2-sparkle-d1 { animation-delay: 0.9s; }
        .df2-sparkle-d2 { animation-delay: 1.8s; }
        .df2-sparkle-d3 { animation-delay: 2.7s; }

        /* ── Hex scroll ── */
        .df2-hex-scroll { animation: dfHexScroll 8s linear infinite; }

        /* ── Ribbon ── */
        .df2-ribbon { animation: dfRibbon 4s linear infinite; }
        .df2-ribbon-d1 { animation-delay: 1.3s; }
        .df2-ribbon-d2 { animation-delay: 2.6s; }

        /* ── Particle travel — curve 1 (blue → purple) ── */
        .df2-p1a { offset-path: path('${curves[0]}'); animation: dfTravel 2.8s ease-in-out infinite; }
        .df2-p1b { offset-path: path('${curves[0]}'); animation: dfTravel 2.8s ease-in-out 0.9s infinite; }
        .df2-p1c { offset-path: path('${curves[0]}'); animation: dfTravel 2.8s ease-in-out 1.8s infinite; }
        .df2-e1  { offset-path: path('${curves[0]}'); animation: dfEnergy 3.6s ease-in-out 0.4s infinite; }

        /* ── Particle travel — curve 2 (purple → green) ── */
        .df2-p2a { offset-path: path('${curves[1]}'); animation: dfTravel 3s ease-in-out infinite; }
        .df2-p2b { offset-path: path('${curves[1]}'); animation: dfTravel 3s ease-in-out 1s infinite; }
        .df2-p2c { offset-path: path('${curves[1]}'); animation: dfTravel 3s ease-in-out 2s infinite; }
        .df2-e2  { offset-path: path('${curves[1]}'); animation: dfEnergy 3.8s ease-in-out 0.6s infinite; }

        /* ── Particle travel — curve 3 (green → amber) ── */
        .df2-p3a { offset-path: path('${curves[2]}'); animation: dfTravel 3.2s ease-in-out infinite; }
        .df2-p3b { offset-path: path('${curves[2]}'); animation: dfTravel 3.2s ease-in-out 1.05s infinite; }
        .df2-p3c { offset-path: path('${curves[2]}'); animation: dfTravel 3.2s ease-in-out 2.1s infinite; }
        .df2-e3  { offset-path: path('${curves[2]}'); animation: dfEnergy 4s ease-in-out 0.8s infinite; }
      `}</style>

      <svg viewBox={`0 0 ${W} ${H}`} className="w-full h-auto" xmlns="http://www.w3.org/2000/svg">
        <defs>
          {/* ── Glow filters ── */}
          <filter id="df2-glow-sm" x="-50%" y="-50%" width="200%" height="200%">
            <feGaussianBlur in="SourceGraphic" stdDeviation="3" />
          </filter>
          <filter id="df2-glow-md" x="-50%" y="-50%" width="200%" height="200%">
            <feGaussianBlur in="SourceGraphic" stdDeviation="6" />
          </filter>
          <filter id="df2-glow-lg" x="-50%" y="-50%" width="200%" height="200%">
            <feGaussianBlur in="SourceGraphic" stdDeviation="10" />
          </filter>

          {/* ── Radial glows ── */}
          <radialGradient id="df2-g-blue" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#3b82f6" stopOpacity="0.45" />
            <stop offset="50%" stopColor="#3b82f6" stopOpacity="0.12" />
            <stop offset="100%" stopColor="#3b82f6" stopOpacity="0" />
          </radialGradient>
          <radialGradient id="df2-g-purple" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#a78bfa" stopOpacity="0.4" />
            <stop offset="50%" stopColor="#a78bfa" stopOpacity="0.1" />
            <stop offset="100%" stopColor="#a78bfa" stopOpacity="0" />
          </radialGradient>
          <radialGradient id="df2-g-green" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#22c55e" stopOpacity="0.5" />
            <stop offset="50%" stopColor="#22c55e" stopOpacity="0.12" />
            <stop offset="100%" stopColor="#22c55e" stopOpacity="0" />
          </radialGradient>
          <radialGradient id="df2-g-amber" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#fbbf24" stopOpacity="0.4" />
            <stop offset="50%" stopColor="#fbbf24" stopOpacity="0.1" />
            <stop offset="100%" stopColor="#fbbf24" stopOpacity="0" />
          </radialGradient>

          {/* ── Inner glass fills ── */}
          <radialGradient id="df2-glass-blue" cx="35%" cy="30%" r="65%">
            <stop offset="0%" stopColor="#60a5fa" stopOpacity="0.12" />
            <stop offset="100%" stopColor="#3b82f6" stopOpacity="0.03" />
          </radialGradient>
          <radialGradient id="df2-glass-purple" cx="35%" cy="30%" r="65%">
            <stop offset="0%" stopColor="#c4b5fd" stopOpacity="0.1" />
            <stop offset="100%" stopColor="#a78bfa" stopOpacity="0.02" />
          </radialGradient>
          <radialGradient id="df2-glass-green" cx="35%" cy="30%" r="65%">
            <stop offset="0%" stopColor="#4ade80" stopOpacity="0.12" />
            <stop offset="100%" stopColor="#22c55e" stopOpacity="0.03" />
          </radialGradient>
          <radialGradient id="df2-glass-amber" cx="35%" cy="30%" r="65%">
            <stop offset="0%" stopColor="#fde68a" stopOpacity="0.1" />
            <stop offset="100%" stopColor="#fbbf24" stopOpacity="0.02" />
          </radialGradient>

          {/* ── Connection gradient strokes ── */}
          <linearGradient id="df2-conn-1" x1="0%" y1="0%" x2="100%" y2="0%">
            <stop offset="0%" stopColor="#3b82f6" />
            <stop offset="100%" stopColor="#a78bfa" />
          </linearGradient>
          <linearGradient id="df2-conn-2" x1="0%" y1="0%" x2="100%" y2="0%">
            <stop offset="0%" stopColor="#a78bfa" />
            <stop offset="100%" stopColor="#22c55e" />
          </linearGradient>
          <linearGradient id="df2-conn-3" x1="0%" y1="0%" x2="100%" y2="0%">
            <stop offset="0%" stopColor="#22c55e" />
            <stop offset="100%" stopColor="#fbbf24" />
          </linearGradient>

          {/* ── Python logo gradients ── */}
          <linearGradient id="df2-py-b" x1="13%" y1="12%" x2="80%" y2="78%">
            <stop stopColor="#387EB8" offset="0%" />
            <stop stopColor="#366994" offset="100%" />
          </linearGradient>
          <linearGradient id="df2-py-y" x1="19%" y1="21%" x2="91%" y2="88%">
            <stop stopColor="#FFC836" offset="0%" />
            <stop stopColor="#FFD43B" offset="100%" />
          </linearGradient>
        </defs>

        {/* ╔══════════════════════════════════════════════════════════╗
            ║  CONNECTION SYSTEM — ribbons, particles, energy         ║
            ╚══════════════════════════════════════════════════════════╝ */}

        {/* Background glow ribbon (blurred) */}
        {curves.map((d, i) => (
          <path key={`bg-${i}`} d={d} fill="none" stroke={`url(#df2-conn-${i + 1})`} strokeWidth="4" opacity="0.06" filter="url(#df2-glow-md)" />
        ))}

        {/* Visible curve paths */}
        {curves.map((d, i) => (
          <path key={`curve-${i}`} d={d} fill="none" stroke={`url(#df2-conn-${i + 1})`} strokeWidth="1.5" opacity="0.1" />
        ))}

        {/* Animated dashed ribbon overlay */}
        {curves.map((d, i) => (
          <path key={`ribbon-${i}`} d={d} fill="none" stroke={`url(#df2-conn-${i + 1})`} strokeWidth="1" opacity="0.15" strokeDasharray="8 12" className={`df2-ribbon${i > 0 ? ` df2-ribbon-d${i}` : ""}`} />
        ))}

        {/* Travelling particles — 3 per curve + 1 energy flash */}
        {/* Curve 1 */}
        <circle r="3.5" fill="#60a5fa" filter="url(#df2-glow-sm)" className="df2-p1a" />
        <circle r="2.5" fill="#93c5fd" className="df2-p1b" />
        <circle r="2" fill="#bfdbfe" className="df2-p1c" />
        <circle r="6" fill="#3b82f6" opacity="0.35" filter="url(#df2-glow-md)" className="df2-e1" />

        {/* Curve 2 */}
        <circle r="3.5" fill="#c4b5fd" filter="url(#df2-glow-sm)" className="df2-p2a" />
        <circle r="2.5" fill="#a78bfa" className="df2-p2b" />
        <circle r="2" fill="#e9d5ff" className="df2-p2c" />
        <circle r="6" fill="#a78bfa" opacity="0.3" filter="url(#df2-glow-md)" className="df2-e2" />

        {/* Curve 3 */}
        <circle r="3.5" fill="#4ade80" filter="url(#df2-glow-sm)" className="df2-p3a" />
        <circle r="2.5" fill="#86efac" className="df2-p3b" />
        <circle r="2" fill="#bbf7d0" className="df2-p3c" />
        <circle r="6" fill="#22c55e" opacity="0.3" filter="url(#df2-glow-md)" className="df2-e3" />


        {/* ╔══════════════════════════════════════════════════════════╗
            ║  NODE 1 — Python Dict                                   ║
            ╚══════════════════════════════════════════════════════════╝ */}
        <g className="df2-node-1 df2-float">
          {/* Outer halo */}
          <circle cx="120" cy={CY} r="105" fill="url(#df2-g-blue)" className="df2-breathe" />
          {/* Secondary halo */}
          <circle cx="120" cy={CY} r="75" fill="url(#df2-g-blue)" className="df2-breathe-soft" />

          {/* Orbiting dashed ring */}
          <g style={{ transformOrigin: `120px ${CY}px` }} className="df2-orbit">
            <circle cx="120" cy={CY} r="56" fill="none" stroke="#3b82f6" strokeWidth="0.4" opacity="0.15" strokeDasharray="3 8" />
          </g>
          <g style={{ transformOrigin: `120px ${CY}px` }} className="df2-orbit-rev">
            <circle cx="120" cy={CY} r="48" fill="none" stroke="#60a5fa" strokeWidth="0.3" opacity="0.1" strokeDasharray="2 12" />
          </g>

          {/* Pulse rings */}
          <circle cx="120" cy={CY} fill="none" stroke="#3b82f6" strokeWidth="0.6" className="df2-pulse" />
          <circle cx="120" cy={CY} fill="none" stroke="#60a5fa" strokeWidth="0.4" className="df2-pulse df2-pulse-d1" />

          {/* Glass core */}
          <circle cx="120" cy={CY} r="44" fill="url(#df2-glass-blue)" />

          {/* Python logo — larger */}
          <g transform={`translate(99, ${CY - 20}) scale(0.16)`}>
            <path d="M126.916.072c-64.832 0-60.784 28.115-60.784 28.115l.072 29.128h61.868v8.745H41.631S.145 61.355.145 126.77c0 65.417 36.21 63.097 36.21 63.097h21.61v-30.356s-1.165-36.21 35.632-36.21h61.362s34.475.557 34.475-33.319V33.97S194.67.072 126.916.072zM92.802 19.66a11.12 11.12 0 0 1 11.13 11.13 11.12 11.12 0 0 1-11.13 11.13 11.12 11.12 0 0 1-11.13-11.13 11.12 11.12 0 0 1 11.13-11.13z" fill="url(#df2-py-b)" />
            <path d="M128.757 254.126c64.832 0 60.784-28.115 60.784-28.115l-.072-29.127H127.6v-8.745h86.441s41.486 4.705 41.486-60.712c0-65.416-36.21-63.096-36.21-63.096h-21.61v30.355s1.165 36.21-35.632 36.21h-61.362s-34.475-.557-34.475 33.32v56.013s-5.235 33.897 62.518 33.897zm34.114-19.586a11.12 11.12 0 0 1-11.13-11.13 11.12 11.12 0 0 1 11.13-11.131 11.12 11.12 0 0 1 11.13 11.13 11.12 11.12 0 0 1-11.13 11.13z" fill="url(#df2-py-y)" />
          </g>

          {/* Floating data fragments — larger, more expressive */}
          <text x="68" y={CY - 42} fontSize="8" fontFamily="monospace" fill="#60a5fa" className="df2-sparkle" opacity="0.5">{"{ }"}</text>
          <text x="148" y={CY - 36} fontSize="7" fontFamily="monospace" fill="#93c5fd" className="df2-sparkle df2-sparkle-d1" opacity="0.4">&quot;name&quot;</text>
          <text x="62" y={CY + 48} fontSize="7" fontFamily="monospace" fill="#fbbf24" className="df2-sparkle df2-sparkle-d2" opacity="0.35">[1, 2]</text>
          <text x="150" y={CY + 42} fontSize="7" fontFamily="monospace" fill="#4ade80" className="df2-sparkle df2-sparkle-d3" opacity="0.4">True</text>
          <text x="80" y={CY + 8} fontSize="6.5" fontFamily="monospace" fill="#a78bfa" className="df2-sparkle df2-sparkle-d1" opacity="0.3">42</text>

          {/* Labels */}
          <text x="120" y={CY + 72} textAnchor="middle" fontSize="13" fontWeight="600" fill="#cbd5e1" fontFamily="Outfit, sans-serif">Python Dict</text>
          <text x="120" y={CY + 88} textAnchor="middle" fontSize="8.5" fill="#4b5563" fontFamily="monospace">input data</text>
        </g>


        {/* ╔══════════════════════════════════════════════════════════╗
            ║  NODE 2 — Type Inspector                                ║
            ╚══════════════════════════════════════════════════════════╝ */}
        <g className="df2-node-2 df2-float df2-float-d1">
          <circle cx="340" cy={CY} r="100" fill="url(#df2-g-purple)" className="df2-breathe df2-breathe-d1" />
          <circle cx="340" cy={CY} r="72" fill="url(#df2-g-purple)" className="df2-breathe-soft" />

          {/* Orbit rings */}
          <g style={{ transformOrigin: `340px ${CY}px` }} className="df2-orbit-rev">
            <circle cx="340" cy={CY} r="52" fill="none" stroke="#a78bfa" strokeWidth="0.4" opacity="0.15" strokeDasharray="4 10" />
          </g>
          <g style={{ transformOrigin: `340px ${CY}px` }} className="df2-orbit-slow">
            <circle cx="340" cy={CY} r="44" fill="none" stroke="#c4b5fd" strokeWidth="0.3" opacity="0.1" strokeDasharray="2 14" />
          </g>

          {/* Pulse rings */}
          <circle cx="340" cy={CY} fill="none" stroke="#a78bfa" strokeWidth="0.5" className="df2-pulse" />
          <circle cx="340" cy={CY} fill="none" stroke="#c4b5fd" strokeWidth="0.3" className="df2-pulse df2-pulse-d1" />

          {/* Glass core */}
          <circle cx="340" cy={CY} r="42" fill="url(#df2-glass-purple)" />

          {/* Scanner concentric rings — subtler and larger */}
          <circle cx="340" cy={CY} r="34" fill="none" stroke="#a78bfa" strokeWidth="0.2" opacity="0.1" />
          <circle cx="340" cy={CY} r="24" fill="none" stroke="#a78bfa" strokeWidth="0.2" opacity="0.15" />
          <circle cx="340" cy={CY} r="14" fill="none" stroke="#c4b5fd" strokeWidth="0.2" opacity="0.2" />

          {/* Crosshair */}
          <line x1="340" y1={CY - 36} x2="340" y2={CY + 36} stroke="#a78bfa" strokeWidth="0.3" opacity="0.1" />
          <line x1={340 - 36} y1={CY} x2={340 + 36} y2={CY} stroke="#a78bfa" strokeWidth="0.3" opacity="0.1" />

          {/* Diagonal scan lines */}
          <line x1={340 - 25} y1={CY - 25} x2={340 + 25} y2={CY + 25} stroke="#c4b5fd" strokeWidth="0.2" opacity="0.06" />
          <line x1={340 + 25} y1={CY - 25} x2={340 - 25} y2={CY + 25} stroke="#c4b5fd" strokeWidth="0.2" opacity="0.06" />

          {/* Type tags — bigger */}
          <text x="322" y={CY - 8} fontSize="9" fontFamily="monospace" fontWeight="700" fill="#c4b5fd" opacity="0.7" className="df2-core-pulse">str</text>
          <text x="345" y={CY + 4} fontSize="9" fontFamily="monospace" fontWeight="700" fill="#a78bfa" opacity="0.6" className="df2-core-pulse">int</text>
          <text x="326" y={CY + 16} fontSize="8" fontFamily="monospace" fontWeight="600" fill="#e9d5ff" opacity="0.45" className="df2-core-pulse">list</text>

          {/* Floating detection badges */}
          <text x="285" y={CY - 40} fontSize="7" fontFamily="monospace" fill="#c4b5fd" className="df2-sparkle" opacity="0.35">dict</text>
          <text x="375" y={CY - 38} fontSize="7" fontFamily="monospace" fill="#a78bfa" className="df2-sparkle df2-sparkle-d2" opacity="0.3">bool</text>
          <text x="290" y={CY + 50} fontSize="6.5" fontFamily="monospace" fill="#e9d5ff" className="df2-sparkle df2-sparkle-d1" opacity="0.25">float</text>
          <text x="375" y={CY + 48} fontSize="6.5" fontFamily="monospace" fill="#c4b5fd" className="df2-sparkle df2-sparkle-d3" opacity="0.3">bytes</text>

          <text x="340" y={CY + 72} textAnchor="middle" fontSize="13" fontWeight="600" fill="#cbd5e1" fontFamily="Outfit, sans-serif">Type Inspector</text>
          <text x="340" y={CY + 88} textAnchor="middle" fontSize="8.5" fill="#4b5563" fontFamily="monospace">introspect</text>
        </g>


        {/* ╔══════════════════════════════════════════════════════════╗
            ║  NODE 3 — C Core Encoder  (the hero node, biggest)      ║
            ╚══════════════════════════════════════════════════════════╝ */}
        <g className="df2-node-3 df2-float df2-float-d2">
          {/* Triple halo — the largest node */}
          <circle cx="580" cy={CY} r="120" fill="url(#df2-g-green)" className="df2-breathe df2-breathe-d2" />
          <circle cx="580" cy={CY} r="90" fill="url(#df2-g-green)" className="df2-breathe-soft" />

          {/* Orbit rings */}
          <g style={{ transformOrigin: `580px ${CY}px` }} className="df2-orbit">
            <circle cx="580" cy={CY} r="62" fill="none" stroke="#22c55e" strokeWidth="0.5" opacity="0.15" strokeDasharray="4 8" />
            {/* Orbit dot */}
            <circle cx={580 + 62} cy={CY} r="2" fill="#4ade80" opacity="0.4" />
          </g>
          <g style={{ transformOrigin: `580px ${CY}px` }} className="df2-orbit-rev">
            <circle cx="580" cy={CY} r="54" fill="none" stroke="#4ade80" strokeWidth="0.3" opacity="0.1" strokeDasharray="3 12" />
            <circle cx={580} cy={CY - 54} r="1.5" fill="#86efac" opacity="0.3" />
          </g>

          {/* Pulse rings */}
          <circle cx="580" cy={CY} fill="none" stroke="#22c55e" strokeWidth="0.6" className="df2-pulse-lg" />
          <circle cx="580" cy={CY} fill="none" stroke="#4ade80" strokeWidth="0.4" className="df2-pulse-lg df2-pulse-lg-d1" />

          {/* Glass core — bigger */}
          <circle cx="580" cy={CY} r="50" fill="url(#df2-glass-green)" />

          {/* Inner organic circuit traces */}
          <path d="M555 178 Q565 172 580 182 Q595 192 600 178" fill="none" stroke="#22c55e" strokeWidth="0.5" opacity="0.2" />
          <path d="M558 205 Q572 214 590 205 Q602 198 612 208" fill="none" stroke="#4ade80" strokeWidth="0.4" opacity="0.15" />
          <path d="M563 192 Q575 188 585 195 Q595 202 608 195" fill="none" stroke="#86efac" strokeWidth="0.3" opacity="0.1" />

          {/* Circuit junction dots */}
          <circle cx="580" cy="182" r="2" fill="#22c55e" opacity="0.4" className="df2-core-pulse" />
          <circle cx="600" cy="205" r="1.5" fill="#4ade80" opacity="0.3" className="df2-core-pulse" />
          <circle cx="563" cy="195" r="1.5" fill="#86efac" opacity="0.25" />

          {/* Core label — prominent */}
          <text x="580" y={CY - 5} textAnchor="middle" fontSize="13" fontFamily="monospace" fontWeight="800" fill="#22c55e" opacity="0.85" className="df2-core-pulse">CROUS</text>
          <text x="580" y={CY + 10} textAnchor="middle" fontSize="8" fontFamily="monospace" fill="#4ade80" opacity="0.45">single-pass encoder</text>

          {/* Floating processing keywords — larger */}
          <text x="520" y={CY - 48} fontSize="7.5" fontFamily="monospace" fill="#4ade80" className="df2-sparkle" opacity="0.35">zigzag</text>
          <text x="618" y={CY - 45} fontSize="7.5" fontFamily="monospace" fill="#86efac" className="df2-sparkle df2-sparkle-d1" opacity="0.3">varint</text>
          <text x="522" y={CY + 55} fontSize="7" fontFamily="monospace" fill="#4ade80" className="df2-sparkle df2-sparkle-d2" opacity="0.25">arena</text>
          <text x="618" y={CY + 52} fontSize="7" fontFamily="monospace" fill="#86efac" className="df2-sparkle df2-sparkle-d3" opacity="0.28">tagged</text>
          <text x="555" y={CY + 30} fontSize="6.5" fontFamily="monospace" fill="#bbf7d0" className="df2-sparkle df2-sparkle-d2" opacity="0.2">zero-copy</text>

          <text x="580" y={CY + 78} textAnchor="middle" fontSize="13" fontWeight="600" fill="#cbd5e1" fontFamily="Outfit, sans-serif">C Core Encoder</text>
          <text x="580" y={CY + 94} textAnchor="middle" fontSize="8.5" fill="#4b5563" fontFamily="monospace">encode</text>
        </g>


        {/* ╔══════════════════════════════════════════════════════════╗
            ║  NODE 4 — FLUX Binary Output                            ║
            ╚══════════════════════════════════════════════════════════╝ */}
        <g className="df2-node-4 df2-float df2-float-d3">
          <circle cx="830" cy={CY} r="105" fill="url(#df2-g-amber)" className="df2-breathe" />
          <circle cx="830" cy={CY} r="75" fill="url(#df2-g-amber)" className="df2-breathe-soft" />

          {/* Orbit */}
          <g style={{ transformOrigin: `830px ${CY}px` }} className="df2-orbit-slow">
            <circle cx="830" cy={CY} r="56" fill="none" stroke="#fbbf24" strokeWidth="0.4" opacity="0.12" strokeDasharray="3 10" />
          </g>

          {/* Pulse */}
          <circle cx="830" cy={CY} fill="none" stroke="#fbbf24" strokeWidth="0.5" className="df2-pulse" />
          <circle cx="830" cy={CY} fill="none" stroke="#fde68a" strokeWidth="0.3" className="df2-pulse df2-pulse-d1" />

          {/* Glass core */}
          <circle cx="830" cy={CY} r="46" fill="url(#df2-glass-amber)" />

          {/* FLUX magic header — bigger */}
          <text x="830" y={CY - 14} textAnchor="middle" fontSize="10" fontFamily="monospace" fontWeight="800" fill="#fbbf24" opacity="0.7">46 4C 55 58</text>
          <text x="830" y={CY} textAnchor="middle" fontSize="8" fontFamily="monospace" fill="#f59e0b" opacity="0.4">FLUX</text>

          {/* Scrolling hex stream — bigger clip region */}
          <clipPath id="df2-hex-clip">
            <rect x={830 - 36} y={CY + 6} width="72" height="32" />
          </clipPath>
          <g clipPath="url(#df2-hex-clip)">
            <g className="df2-hex-scroll">
              <text x="830" y={CY + 18} textAnchor="middle" fontSize="6.5" fontFamily="monospace" fill="#fbbf24" opacity="0.3">0A 05 6E 61</text>
              <text x="830" y={CY + 30} textAnchor="middle" fontSize="6.5" fontFamily="monospace" fill="#fbbf24" opacity="0.22">6D 65 03 41</text>
              <text x="830" y={CY + 42} textAnchor="middle" fontSize="6.5" fontFamily="monospace" fill="#fbbf24" opacity="0.3">6C 69 63 65</text>
              <text x="830" y={CY + 54} textAnchor="middle" fontSize="6.5" fontFamily="monospace" fill="#fbbf24" opacity="0.22">C4 01 BE 01</text>
              <text x="830" y={CY + 66} textAnchor="middle" fontSize="6.5" fontFamily="monospace" fill="#fbbf24" opacity="0.3">FF 08 A2 9C</text>
              <text x="830" y={CY + 78} textAnchor="middle" fontSize="6.5" fontFamily="monospace" fill="#fbbf24" opacity="0.22">0A 05 6E 61</text>
            </g>
          </g>

          {/* Size badge */}
          <text x="830" y={CY + 48} textAnchor="middle" fontSize="8.5" fontFamily="monospace" fontWeight="700" fill="#fbbf24" opacity="0.5" className="df2-core-pulse">42 B</text>

          <text x="830" y={CY + 72} textAnchor="middle" fontSize="13" fontWeight="600" fill="#cbd5e1" fontFamily="Outfit, sans-serif">FLUX Binary</text>
          <text x="830" y={CY + 88} textAnchor="middle" fontSize="8.5" fill="#4b5563" fontFamily="monospace">output</text>
        </g>


        {/* ╔══════════════════════════════════════════════════════════╗
            ║  STEP INDICATORS                                        ║
            ╚══════════════════════════════════════════════════════════╝ */}
        <g>
          {[
            { x: 120, n: "1", c: "#3b82f6" },
            { x: 340, n: "2", c: "#a78bfa" },
            { x: 580, n: "3", c: "#22c55e" },
            { x: 830, n: "4", c: "#fbbf24" },
          ].map((s) => (
            <g key={s.n}>
              <circle cx={s.x} cy={CY + 112} r="12" fill={s.c} opacity="0.06" />
              <circle cx={s.x} cy={CY + 112} r="6" fill={s.c} opacity="0.04" />
              <text x={s.x} y={CY + 116} textAnchor="middle" fontSize="9" fontWeight="700" fill={s.c} opacity="0.55" fontFamily="monospace">{s.n}</text>
            </g>
          ))}
          {/* Connecting lines between step dots */}
          <line x1="136" y1={CY + 112} x2="324" y2={CY + 112} stroke="#27272a" strokeWidth="0.5" opacity="0.2" />
          <line x1="356" y1={CY + 112} x2="564" y2={CY + 112} stroke="#27272a" strokeWidth="0.5" opacity="0.2" />
          <line x1="596" y1={CY + 112} x2="814" y2={CY + 112} stroke="#27272a" strokeWidth="0.5" opacity="0.2" />
        </g>

        {/* ═══ Directional arrows — subtle gradient triangles ═══ */}
        <g>
          <polygon points="220,{CY-8} 232,{CY} 220,{CY+8}" fill="url(#df2-conn-1)" opacity="0.12" />
          <polygon points={`220,${CY - 8} 232,${CY} 220,${CY + 8}`} fill="url(#df2-conn-1)" opacity="0.12" />
          <polygon points={`440,${CY - 8} 452,${CY} 440,${CY + 8}`} fill="url(#df2-conn-2)" opacity="0.12" />
          <polygon points={`695,${CY - 8} 707,${CY} 695,${CY + 8}`} fill="url(#df2-conn-3)" opacity="0.12" />
        </g>
      </svg>
    </div>
  );
}


/* ─── Module Architecture ─────────────────────────────────────── */
export function ArchitectureDiagram() {
  return (
    <div className="relative w-full overflow-hidden">
      <style>{`
        @keyframes fadeUp {
          0% { opacity: 0; transform: translateY(12px); }
          100% { opacity: 1; transform: translateY(0); }
        }
        @keyframes softPulse {
          0%, 100% { opacity: 0.5; }
          50% { opacity: 0.9; }
        }
        @keyframes flowDown {
          0% { offset-distance: 0%; opacity: 0; }
          10% { opacity: 0.8; }
          90% { opacity: 0.8; }
          100% { offset-distance: 100%; opacity: 0; }
        }
        @keyframes glowBreath {
          0%, 100% { opacity: 0.12; }
          50% { opacity: 0.25; }
        }
        @keyframes ringExpand {
          0% { r: 20; opacity: 0.2; }
          100% { r: 35; opacity: 0; }
        }
        .ar-row-1 { animation: fadeUp 0.7s ease-out 0.1s both; }
        .ar-row-2 { animation: fadeUp 0.7s ease-out 0.25s both; }
        .ar-row-3 { animation: fadeUp 0.7s ease-out 0.4s both; }
        .ar-row-4 { animation: fadeUp 0.7s ease-out 0.55s both; }
        .ar-soft-pulse { animation: softPulse 3s ease-in-out infinite; }
        .ar-soft-pulse-d1 { animation-delay: 0.5s; }
        .ar-soft-pulse-d2 { animation-delay: 1s; }
        .ar-glow-breath { animation: glowBreath 4s ease-in-out infinite; }
        .ar-ring-expand { animation: ringExpand 3s ease-out infinite; }
        .ar-flow-1 { offset-path: path('M290 95 C290 110 290 120 290 140'); animation: flowDown 2.5s ease-in-out infinite; }
        .ar-flow-2 { offset-path: path('M610 95 C610 110 610 120 610 140'); animation: flowDown 2.5s ease-in-out 0.8s infinite; }
        .ar-flow-3 { offset-path: path('M450 230 C450 245 450 260 450 275'); animation: flowDown 2.8s ease-in-out 0.3s infinite; }
        .ar-flow-4 { offset-path: path('M450 355 C450 370 450 380 450 395'); animation: flowDown 2.8s ease-in-out 0.6s infinite; }
      `}</style>

      <svg viewBox="0 0 900 470" className="w-full h-auto" xmlns="http://www.w3.org/2000/svg">
        <defs>
          <radialGradient id="ar-glow-blue" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#3b82f6" stopOpacity="0.25" />
            <stop offset="100%" stopColor="#3b82f6" stopOpacity="0" />
          </radialGradient>
          <radialGradient id="ar-glow-node" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#539E43" stopOpacity="0.25" />
            <stop offset="100%" stopColor="#539E43" stopOpacity="0" />
          </radialGradient>
          <radialGradient id="ar-glow-green" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#22c55e" stopOpacity="0.2" />
            <stop offset="100%" stopColor="#22c55e" stopOpacity="0" />
          </radialGradient>
          <radialGradient id="ar-glow-gold" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#fbbf24" stopOpacity="0.2" />
            <stop offset="100%" stopColor="#fbbf24" stopOpacity="0" />
          </radialGradient>
          <radialGradient id="ar-glow-purple" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#a78bfa" stopOpacity="0.2" />
            <stop offset="100%" stopColor="#a78bfa" stopOpacity="0" />
          </radialGradient>
          <radialGradient id="ar-glow-core" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#22c55e" stopOpacity="0.15" />
            <stop offset="60%" stopColor="#22c55e" stopOpacity="0.04" />
            <stop offset="100%" stopColor="#22c55e" stopOpacity="0" />
          </radialGradient>
          <radialGradient id="ar-glow-orange" cx="50%" cy="50%" r="50%">
            <stop offset="0%" stopColor="#f97316" stopOpacity="0.2" />
            <stop offset="100%" stopColor="#f97316" stopOpacity="0" />
          </radialGradient>

          <linearGradient id="ar-py-blue" x1="12.959%" y1="12.039%" x2="79.639%" y2="78.201%">
            <stop stopColor="#387EB8" offset="0%" />
            <stop stopColor="#366994" offset="100%" />
          </linearGradient>
          <linearGradient id="ar-py-yellow" x1="19.128%" y1="20.579%" x2="90.742%" y2="88.429%">
            <stop stopColor="#FFC836" offset="0%" />
            <stop stopColor="#FFD43B" offset="100%" />
          </linearGradient>
        </defs>

        {/* ═══ Vertical flow lines (faint background) ═══ */}
        <line x1="290" y1="95" x2="290" y2="145" stroke="#3b82f6" strokeWidth="0.5" opacity="0.06" />
        <line x1="610" y1="95" x2="610" y2="145" stroke="#539E43" strokeWidth="0.5" opacity="0.06" />
        <line x1="450" y1="230" x2="450" y2="280" stroke="#22c55e" strokeWidth="0.5" opacity="0.06" />
        <line x1="450" y1="355" x2="450" y2="400" stroke="#22c55e" strokeWidth="0.5" opacity="0.06" />

        {/* Animated flow dots */}
        <circle r="2.5" fill="#60a5fa" className="ar-flow-1" />
        <circle r="2.5" fill="#539E43" className="ar-flow-2" />
        <circle r="2.5" fill="#4ade80" className="ar-flow-3" />
        <circle r="2" fill="#4ade80" className="ar-flow-4" />

        {/* ═══ Row 1: SDK Bindings ═══ */}
        <g className="ar-row-1">
          <text x="80" y="55" fontSize="9" fontWeight="700" fill="#4b5563" fontFamily="Outfit, sans-serif" letterSpacing="3">SDK BINDINGS</text>
          <line x1="80" y1="62" x2="195" y2="62" stroke="#27272a" strokeWidth="0.5" />

          {/* Python SDK node */}
          <g>
            <circle cx="290" cy="65" r="55" fill="url(#ar-glow-blue)" className="ar-glow-breath" />
            <circle cx="290" cy="65" r="24" fill="#3b82f6" opacity="0.05" />

            <g transform="translate(277, 50) scale(0.1)">
              <path d="M126.916.072c-64.832 0-60.784 28.115-60.784 28.115l.072 29.128h61.868v8.745H41.631S.145 61.355.145 126.77c0 65.417 36.21 63.097 36.21 63.097h21.61v-30.356s-1.165-36.21 35.632-36.21h61.362s34.475.557 34.475-33.319V33.97S194.67.072 126.916.072zM92.802 19.66a11.12 11.12 0 0 1 11.13 11.13 11.12 11.12 0 0 1-11.13 11.13 11.12 11.12 0 0 1-11.13-11.13 11.12 11.12 0 0 1 11.13-11.13z" fill="url(#ar-py-blue)" />
              <path d="M128.757 254.126c64.832 0 60.784-28.115 60.784-28.115l-.072-29.127H127.6v-8.745h86.441s41.486 4.705 41.486-60.712c0-65.416-36.21-63.096-36.21-63.096h-21.61v30.355s1.165 36.21-35.632 36.21h-61.362s-34.475-.557-34.475 33.32v56.013s-5.235 33.897 62.518 33.897zm34.114-19.586a11.12 11.12 0 0 1-11.13-11.13 11.12 11.12 0 0 1 11.13-11.131 11.12 11.12 0 0 1 11.13 11.13 11.12 11.12 0 0 1-11.13 11.13z" fill="url(#ar-py-yellow)" />
            </g>

            <text x="290" y="102" textAnchor="middle" fontSize="10" fontWeight="600" fill="#e2e8f0" fontFamily="Outfit, sans-serif">Python SDK</text>
            <text x="290" y="115" textAnchor="middle" fontSize="7" fill="#4b5563" fontFamily="monospace">pycrous.c · C-API</text>
          </g>

          {/* Node.js SDK node */}
          <g>
            <circle cx="610" cy="65" r="55" fill="url(#ar-glow-node)" className="ar-glow-breath" />
            <circle cx="610" cy="65" r="24" fill="#539E43" opacity="0.05" />

            <g transform="translate(598, 48) scale(0.1)">
              <path d="M128 288.464c-3.975 0-7.685-1.06-11.13-2.915l-35.247-20.936c-5.3-2.915-2.65-3.975-1.06-4.505 7.155-2.385 8.48-2.915 15.9-7.156.795-.53 1.855-.265 2.65.265l27.032 16.166c1.06.53 2.385.53 3.18 0l105.74-61.216c1.06-.53 1.59-1.59 1.59-2.915V83.082c0-1.325-.53-2.385-1.59-2.915L129.325 19.167c-1.06-.53-2.385-.53-3.18 0L20.405 80.432c-1.06.53-1.59 1.855-1.59 2.915v122.17c0 1.06.53 2.385 1.59 2.915l28.887 16.695c15.636 7.95 25.44-1.325 25.44-10.6V93.152c0-1.59 1.325-3.18 3.18-3.18h13.515c1.59 0 3.18 1.325 3.18 3.18v121.376c0 20.936-11.395 33.126-31.27 33.126-6.095 0-10.865 0-24.38-6.625l-27.827-15.9C4.505 221.42 0 214 0 206.052V83.082c0-7.95 4.505-15.37 11.925-19.346L117.665 2.52c7.155-3.975 16.695-3.975 23.85 0l105.74 61.216C254.675 67.711 259.18 75.131 259.18 83.082v122.17c0 7.95-4.505 15.37-11.925 19.345l-105.74 61.216c-3.445 1.855-7.42 2.65-11.395 2.65h-.12z" fill="#539E43" />
              <path d="M160.33 205.257c-44.78 0-54.12-20.67-54.12-37.896 0-1.59 1.325-3.18 3.18-3.18h13.78c1.59 0 2.915 1.06 2.915 2.65 2.12 14.045 8.215 20.936 36.305 20.936 22.26 0 31.8-5.035 31.8-16.96 0-6.89-2.65-11.925-37.365-15.37-28.887-2.915-46.907-9.275-46.907-32.33 0-21.466 18.02-34.186 48.232-34.186 33.92 0 50.615 11.66 52.735 37.1 0 .795-.265 1.59-.795 2.385-.53.53-1.325 1.06-2.12 1.06h-13.78c-1.325 0-2.65-1.06-2.915-2.385-3.18-14.575-11.395-19.345-33.125-19.345-24.38 0-27.3 8.48-27.3 14.84 0 7.685 3.445 10.07 36.305 14.31 32.595 4.24 47.967 10.335 47.967 33.126-.265 23.32-19.345 36.57-53.06 36.57l.268-.325z" fill="#539E43" />
            </g>

            <text x="610" y="102" textAnchor="middle" fontSize="10" fontWeight="600" fill="#e2e8f0" fontFamily="Outfit, sans-serif">Node.js SDK</text>
            <text x="610" y="115" textAnchor="middle" fontSize="7" fill="#4b5563" fontFamily="monospace">N-API · ABI stable</text>
          </g>
        </g>

        {/* ═══ Row 2: Value Core ═══ */}
        <g className="ar-row-2">
          <text x="80" y="175" fontSize="9" fontWeight="700" fill="#4b5563" fontFamily="Outfit, sans-serif" letterSpacing="3">VALUE CORE</text>
          <line x1="80" y1="182" x2="175" y2="182" stroke="#27272a" strokeWidth="0.5" />

          <circle cx="450" cy="190" r="90" fill="url(#ar-glow-core)" className="ar-glow-breath" />
          <circle cx="450" cy="190" fill="none" stroke="#22c55e" strokeWidth="0.3" className="ar-ring-expand" />

          <text x="450" y="172" textAnchor="middle" fontSize="10" fontWeight="700" fill="#22c55e" fontFamily="monospace" opacity="0.7" className="ar-soft-pulse">CrousValue</text>
          <text x="450" y="185" textAnchor="middle" fontSize="7" fill="#4ade80" fontFamily="monospace" opacity="0.35">tagged union · 10 types</text>

          {/* Type constellation */}
          {[
            { label: "null", x: 185, y: 205, color: "#6b7280" },
            { label: "bool", x: 248, y: 200, color: "#22c55e" },
            { label: "int", x: 310, y: 210, color: "#3b82f6" },
            { label: "float", x: 370, y: 205, color: "#60a5fa" },
            { label: "str", x: 430, y: 212, color: "#a78bfa" },
            { label: "bytes", x: 490, y: 208, color: "#c4b5fd" },
            { label: "list", x: 550, y: 212, color: "#f97316" },
            { label: "tuple", x: 610, y: 205, color: "#fb923c" },
            { label: "dict", x: 670, y: 210, color: "#fbbf24" },
            { label: "tagged", x: 740, y: 205, color: "#22c55e" },
          ].map((t, i) => (
            <g key={t.label}>
              <circle cx={t.x} cy={t.y} r="3" fill={t.color} opacity="0.2" />
              <circle cx={t.x} cy={t.y} r="1.5" fill={t.color} opacity="0.5" />
              <text x={t.x} y={t.y + 14} textAnchor="middle" fontSize="7.5" fontFamily="monospace" fontWeight="500" fill={t.color} opacity={0.5 + (i % 3) * 0.1}>
                {t.label}
              </text>
            </g>
          ))}
        </g>

        {/* ═══ Row 3: Encoding Engine ═══ */}
        <g className="ar-row-3">
          <text x="80" y="300" fontSize="9" fontWeight="700" fill="#4b5563" fontFamily="Outfit, sans-serif" letterSpacing="3">ENCODING</text>
          <line x1="80" y1="307" x2="160" y2="307" stroke="#27272a" strokeWidth="0.5" />

          {/* FLUX Binary */}
          <g>
            <circle cx="250" cy="325" r="50" fill="url(#ar-glow-gold)" className="ar-glow-breath" />
            <circle cx="250" cy="325" r="22" fill="#fbbf24" opacity="0.04" />
            <text x="250" y="322" textAnchor="middle" fontSize="9" fontWeight="700" fill="#fbbf24" fontFamily="monospace" opacity="0.7" className="ar-soft-pulse">FLUX</text>
            <text x="250" y="334" textAnchor="middle" fontSize="6.5" fill="#f59e0b" fontFamily="monospace" opacity="0.35">binary codec</text>
            <text x="250" y="360" textAnchor="middle" fontSize="7" fill="#92400e" fontFamily="monospace" opacity="0.3">encode · decode · stream</text>
          </g>

          {/* CROUT Text */}
          <g>
            <circle cx="450" cy="325" r="50" fill="url(#ar-glow-purple)" className="ar-glow-breath" />
            <circle cx="450" cy="325" r="22" fill="#a78bfa" opacity="0.04" />
            <text x="450" y="322" textAnchor="middle" fontSize="9" fontWeight="700" fill="#a78bfa" fontFamily="monospace" opacity="0.7" className="ar-soft-pulse ar-soft-pulse-d1">CROUT</text>
            <text x="450" y="334" textAnchor="middle" fontSize="6.5" fill="#8b5cf6" fontFamily="monospace" opacity="0.35">text format</text>
            <text x="450" y="360" textAnchor="middle" fontSize="7" fill="#5b21b6" fontFamily="monospace" opacity="0.3">lexer · parser · emitter</text>
          </g>

          {/* Varint Engine */}
          <g>
            <circle cx="650" cy="325" r="50" fill="url(#ar-glow-orange)" className="ar-glow-breath" />
            <circle cx="650" cy="325" r="22" fill="#f97316" opacity="0.04" />
            <text x="650" y="322" textAnchor="middle" fontSize="9" fontWeight="700" fill="#f97316" fontFamily="monospace" opacity="0.7" className="ar-soft-pulse ar-soft-pulse-d2">Varint</text>
            <text x="650" y="334" textAnchor="middle" fontSize="6.5" fill="#ea580c" fontFamily="monospace" opacity="0.35">int encoding</text>
            <text x="650" y="360" textAnchor="middle" fontSize="7" fill="#9a3412" fontFamily="monospace" opacity="0.3">zigzag · LEB128</text>
          </g>

          {/* Subtle arcs */}
          <path d="M275 325 Q350 295 425 325" fill="none" stroke="#a78bfa" strokeWidth="0.3" opacity="0.08" />
          <path d="M475 325 Q560 295 625 325" fill="none" stroke="#f97316" strokeWidth="0.3" opacity="0.08" />
        </g>

        {/* ═══ Row 4: Foundation ═══ */}
        <g className="ar-row-4">
          <text x="80" y="425" fontSize="9" fontWeight="700" fill="#4b5563" fontFamily="Outfit, sans-serif" letterSpacing="3">FOUNDATION</text>
          <line x1="80" y1="432" x2="185" y2="432" stroke="#27272a" strokeWidth="0.5" />

          {[
            { label: "Arena Allocator", x: 260, color: "#22c55e" },
            { label: "Error Handling", x: 400, color: "#ef4444" },
            { label: "Buffer I/O", x: 540, color: "#3b82f6" },
            { label: "Type Registry", x: 680, color: "#a78bfa" },
          ].map((f) => (
            <g key={f.label}>
              <circle cx={f.x} cy="445" r="3" fill={f.color} opacity="0.15" />
              <circle cx={f.x} cy="445" r="1.5" fill={f.color} opacity="0.4" />
              <text x={f.x + 10} y="449" fontSize="8" fontWeight="500" fill={f.color} fontFamily="monospace" opacity="0.45">{f.label}</text>
            </g>
          ))}

          <line x1="240" y1="460" x2="800" y2="460" stroke="#22c55e" strokeWidth="0.3" opacity="0.08" />
        </g>

        {/* ═══ Faint horizontal connections ═══ */}
        <line x1="290" y1="120" x2="610" y2="120" stroke="#27272a" strokeWidth="0.3" opacity="0.08" strokeDasharray="4 8" />
        <line x1="250" y1="350" x2="650" y2="350" stroke="#27272a" strokeWidth="0.3" opacity="0.06" strokeDasharray="4 8" />
      </svg>
    </div>
  );
}
