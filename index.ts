import { McpAgent } from "agents/mcp";
import { McpServer } from "@modelcontextprotocol/sdk/server/mcp.js";
import { z } from "zod";
import landingHtml from "./index.html";

export class AigapMCP extends McpAgent {
  server = new McpServer({ name: "aigap", version: "0.1.0" });

  async init() {
    this.server.tool(
      "hello",
      "Returns a greeting",
      { name: z.string() },
      async ({ name }) => ({
        content: [{ type: "text", text: `Hello, ${name} from mcp.aigap.no` }]
      })
    );
  }
}

export default {
  fetch(request: Request, env: Env, ctx: ExecutionContext) {
    const url = new URL(request.url);
    if (url.pathname === "/mcp") {
      return AigapMCP.serve("/mcp").fetch(request, env, ctx);
    }
    if (url.pathname === "/" || url.pathname === "/index.html") {
      return new Response(landingHtml, {
        headers: { "content-type": "text/html; charset=utf-8" }
      });
    }
    return new Response("Not found", { status: 404 });
  }
};