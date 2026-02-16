import { Info, AlertTriangle, AlertCircle, CheckCircle } from "lucide-react";

interface CalloutProps {
  type: "info" | "warning" | "danger" | "success";
  title?: string;
  children: React.ReactNode;
}

const icons = {
  info: Info,
  warning: AlertTriangle,
  danger: AlertCircle,
  success: CheckCircle,
};

const styles = {
  info: "callout-info",
  warning: "callout-warning",
  danger: "callout-danger",
  success: "callout-success",
};

const iconColors = {
  info: "text-blue-400",
  warning: "text-yellow-400",
  danger: "text-red-400",
  success: "text-crous-400",
};

const titleColors = {
  info: "text-blue-300",
  warning: "text-yellow-300",
  danger: "text-red-300",
  success: "text-crous-300",
};

export function Callout({ type, title, children }: CalloutProps) {
  const Icon = icons[type];

  return (
    <div className={styles[type]}>
      <div className="flex items-start gap-3">
        <Icon className={`w-5 h-5 mt-0.5 flex-shrink-0 ${iconColors[type]}`} />
        <div className="flex-1 min-w-0">
          {title && (
            <p className={`font-bold text-sm mb-1 ${titleColors[type]}`}>
              {title}
            </p>
          )}
          <div className="text-sm text-gray-300">{children}</div>
        </div>
      </div>
    </div>
  );
}
