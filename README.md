# libarchive-fuzzer

A custom mutation-based fuzzer built in C for discovering crashes in 
:contentReference[oaicite:0]{index=0} libarchive archive parsers.

This project was created to improve my vulnerability research and exploit development skills by targeting real-world userland software instead of toy binaries.

---

## Motivation

Most beginner fuzzing projects target intentionally vulnerable programs.

I wanted to move toward real-world vulnerability research by:

- Building my own mutation engine
- Generating malformed archive files
- Fuzzing a production-grade parser
- Detecting crashes
- Performing crash triage/debugging

Target:

```bash
bsdtar -tf <archive_file>