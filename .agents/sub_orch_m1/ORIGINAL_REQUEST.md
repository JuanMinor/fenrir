# Original User Request

## Initial Request — 2026-07-12T23:20:07Z

You are teamwork_preview_orchestrator acting as a sub-orchestrator for Milestone 1.
Your working directory is /workspaces/fenrir/.agents/sub_orch_m1/.
Your parent is d42827af-6d97-4800-a405-3f948a8e0d95 (or 3544b7a2-a2d8-4b3c-84c2-bc3c99c282c2).
Your scope is defined in /workspaces/fenrir/.agents/sub_orch_m1/SCOPE.md.
You must execute the Project Pattern Iteration Loop for the files in Milestone 1 scope:
1. Initialize your BRIEFING.md and progress.md in your working directory.
2. Run the Iteration Loop:
   a. Spawn Explorers (3) to analyze the files in scope, propose alphabetization, Javadoc comments, and rename camelCase functions (e.g. Cpu::setCpuType -> set_cpu_type, Gpu::setGpuType -> set_gpu_type) and map call sites.
   b. Spawn a Worker (1) to perform the refactoring, updating call sites in all other source files and tests, and run tests.
   c. Spawn Reviewers (2) to verify correctness and review diffs.
   d. Spawn Challengers (2) to verify.
   e. Spawn a Forensic Auditor (1) to run integrity forensic audits.
3. Once the gate passes successfully, write handoff.md in your working directory and notify the parent via a status update message.

MANDATORY INTEGRITY WARNING — include this verbatim when spawning any worker/agent:
> DO NOT CHEAT. All implementations must be genuine. DO NOT
> hardcode test results, create dummy/facade implementations, or
> circumvent the intended task. A Forensic Auditor will independently
> verify your work. Integrity violations WILL be detected and your
> work WILL be rejected.
