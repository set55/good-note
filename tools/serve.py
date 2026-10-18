#!/usr/bin/env python3
"""在本機開 HTML 版筆記，並讓作答框的「儲存」把答案寫回 .md 原稿。

用法：python3 tools/serve.py [port]（預設 8000），然後開 http://localhost:8000
只聽 127.0.0.1；只接受寫 small-project/ 與 review/ 底下的 .md；寫完會重跑 build_html.py。
"""
import datetime
import json
import os
import re
import sys
import threading
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import build_html  # noqa: E402

ROOT = build_html.ROOT
LOCK = threading.Lock()
FENCE_OPEN = re.compile(r"^(\s*)(`{3,}|~{3,})\s*$")


class SaveError(Exception):
    def __init__(self, status, msg):
        super().__init__(msg)
        self.status = status


def fence_content(lines, start, indent, fence):
    """回傳 (結束行號, 內容)；內容的算法跟 markdown-it 一樣：去掉縮排、每行接 \\n。"""
    for end in range(start + 1, len(lines)):
        s = lines[end]
        if s.startswith(indent) and s[len(indent):].rstrip() and set(s[len(indent):].rstrip()) == {fence[0]} \
                and len(s[len(indent):].rstrip()) >= len(fence):
            body = [l[len(indent):] if l.startswith(indent) else l.lstrip() for l in lines[start + 1:end]]
            return end, "".join(l + "\n" for l in body)
    raise SaveError(409, "找不到作答框的結尾，請重新整理頁面")


def save_answer(rel, line, original, content):
    path = (ROOT / rel).resolve()
    try:
        rel_path = path.relative_to(ROOT)
    except ValueError:
        raise SaveError(403, "只能寫 repo 裡的檔案")
    if rel_path.parts[0] not in build_html.PRACTICE_DIRS or path.suffix != ".md" or not path.is_file():
        raise SaveError(403, "只能寫 small-project/ 與 review/ 底下的 .md")

    text = path.read_text(encoding="utf-8")
    lines = text.split("\n")
    m = FENCE_OPEN.match(lines[line]) if 0 <= line < len(lines) else None
    if not m:
        raise SaveError(409, "檔案在別處被改過（例如已經批改），請先把框裡的內容複製起來，再重新整理頁面")
    indent, fence = m.group(1), m.group(2)
    end, current = fence_content(lines, line, indent, fence)
    norm = lambda s: s.replace("\r\n", "\n").rstrip("\n")
    if norm(current) != norm(original):
        raise SaveError(409, "這一格在別處被改過（例如直接改了 .md），請先把框裡的內容複製起來，再重新整理頁面")

    body = norm(content)
    longest = max((len(r) for r in re.findall(r"`+", body)), default=0)
    new_fence = "`" * max(3, longest + 1)  # 答案裡有 ``` 時，外框要比它長
    new = [indent + new_fence]
    if body:
        new += [indent + l if l else "" for l in body.split("\n")]
    new.append(indent + new_fence)
    lines[line:end + 1] = new

    # 作答標籤還是 `YYYY-MM-DD` 的，存檔時填上今天
    k = line - 1
    while k >= 0 and not lines[k].strip():
        k -= 1
    for j in (k, k - 1, k - 2):
        if j >= 0 and "YYYY-MM-DD" in lines[j] and lines[j].lstrip().startswith("**第"):
            lines[j] = lines[j].replace("YYYY-MM-DD", datetime.date.today().isoformat())
            break

    tmp = path.with_suffix(".md.tmp")
    tmp.write_text("\n".join(lines), encoding="utf-8")
    os.replace(tmp, path)
    return len(new) - (end + 1 - line)


class Handler(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Cache-Control", "no-store")
        super().end_headers()

    def do_GET(self):
        if self.path in ("/", "/index.html"):
            self.send_response(302)
            self.send_header("Location", "/html/index.html")
            self.end_headers()
            return
        super().do_GET()

    def reply(self, status, obj):
        data = json.dumps(obj, ensure_ascii=False).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        self.wfile.write(data)

    def do_POST(self):
        if self.path != "/api/save":
            return self.reply(404, {"error": "沒有這個 API"})
        # 擋掉其他網站對 localhost 發的請求：Host 只能是本機名稱（防 DNS rebinding）、Origin 要同源、只收 JSON
        port = self.server.server_port
        host = self.headers.get("Host", "")
        origin = self.headers.get("Origin")
        if host not in (f"localhost:{port}", f"127.0.0.1:{port}") or (origin and origin != f"http://{host}"):
            return self.reply(403, {"error": "不接受其他網站的請求"})
        if not self.headers.get("Content-Type", "").startswith("application/json"):
            return self.reply(415, {"error": "要用 JSON"})
        try:
            req = json.loads(self.rfile.read(int(self.headers.get("Content-Length", 0))))
            with LOCK:
                delta = save_answer(req["file"], int(req["line"]), req["original"], req["content"])
                build_html.main()
            self.log_message("已寫入 %s 第 %d 行", req["file"], int(req["line"]) + 1)
            self.reply(200, {"ok": True, "delta": delta})
        except SaveError as e:
            self.reply(e.status, {"error": str(e)})
        except (KeyError, ValueError, json.JSONDecodeError) as e:
            self.reply(400, {"error": f"請求格式錯誤：{e}"})


def main():
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 8000
    with LOCK:
        build_html.main()
    server = ThreadingHTTPServer(("127.0.0.1", port), partial(Handler, directory=str(ROOT)))
    print(f"開啟 http://localhost:{port}/ （Ctrl+C 結束）")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
