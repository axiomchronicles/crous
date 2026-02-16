import { Outlet } from "react-router";
import { Navbar } from "~/components/Navbar";
import { Footer } from "~/components/Footer";

export default function DocsLayout() {
  return (
    <div className="min-h-screen flex flex-col">
      <Navbar />
      <div className="flex-1 pt-16">
        <Outlet />
      </div>
      <Footer />
    </div>
  );
}
