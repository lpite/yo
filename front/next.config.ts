import type { NextConfig } from "next";

const API_URL = process.env.NEXT_PUBLIC_API_URL ?? "http://localhost:3000";

const nextConfig: NextConfig = {
  async rewrites() {
    return [
      {
        source: "/sensors",
        destination: `${API_URL}/sensors`,
      },
      {
        source: "/sensors/:path*",
        destination: `${API_URL}/sensors/:path*`,
      },
    ];
  },
};

export default nextConfig;
