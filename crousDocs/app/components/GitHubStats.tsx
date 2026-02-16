import { useState, useEffect } from "react";
import { Star, GitFork } from "lucide-react";

interface GitHubStatsData {
  stars: number;
  forks: number;
}

export function GitHubStats() {
  const [stats, setStats] = useState<GitHubStatsData | null>(null);

  useEffect(() => {
    fetch("https://api.github.com/repos/axiomchronicles/crous")
      .then((r) => r.json())
      .then((data) => {
        if (data.stargazers_count !== undefined) {
          setStats({
            stars: data.stargazers_count,
            forks: data.forks_count,
          });
        }
      })
      .catch(() => {
        /* fail silently — badge just won't show */
      });
  }, []);

  if (!stats) return null;

  return (
    <span className="inline-flex items-center gap-2 ml-1">
      <span className="inline-flex items-center gap-1 text-xs text-gray-500">
        <Star className="w-3 h-3 fill-yellow-500/80 text-yellow-500/80" />
        {stats.stars}
      </span>
      {stats.forks > 0 && (
        <span className="inline-flex items-center gap-1 text-xs text-gray-500">
          <GitFork className="w-3 h-3" />
          {stats.forks}
        </span>
      )}
    </span>
  );
}
