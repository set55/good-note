#!/usr/bin/env python3
"""把筆記的 .md 轉成 html/ 底下的 .html（.md 是原稿，html/ 是產物）。

用法：python3 tools/build_html.py
不轉的：tools/、html/、CLAUDE.md、learning/ 的兩個追蹤檔。
small-project/ 與 review/ 的作答框會變成輸入框，透過 tools/serve.py 寫回 .md。
"""
import html
import json
import os
import re
import shutil
import sys
import unicodedata
from pathlib import Path
from urllib.parse import quote, unquote

from markdown_it import MarkdownIt
from markdown_it.token import Token
from pygments import highlight
from pygments.formatters import HtmlFormatter
from pygments.lexers import get_lexer_by_name
from pygments.util import ClassNotFound

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "html"
SKIP_DIRS = {"tools", "html", ".git"}
PRACTICE_DIRS = ("small-project", "review")  # 作答區，不是筆記領域
SKIP_FILES = {"CLAUDE.md", "learning/學習進度.md", "learning/複習排程.md"}
LANG_ALIAS = {"asm": "gas", "sh": "bash", "zsh": "bash", "jsonc": "json", "console": "console"}


def note_files():
    files = []
    for p in sorted(ROOT.rglob("*.md")):
        rel = p.relative_to(ROOT)
        if rel.parts[0] in SKIP_DIRS or rel.as_posix() in SKIP_FILES or len(rel.parts) == 1:
            continue
        files.append(rel)
    return files


# ---------- GitHub 風格的 heading id ----------

def slugify(text):
    text = text.strip().lower()
    out = []
    for ch in text:
        cat = unicodedata.category(ch)
        if ch in "-_" or cat[0] in "LNM":
            out.append(ch)
        elif ch == " ":
            out.append("-")
    return "".join(out)


def inline_text(token):
    return "".join(c.content for c in (token.children or []) if c.type in ("text", "code_inline"))


# ---------- markdown → html ----------

def highlight_code(code, lang, attrs):
    lang = (lang or "").strip().split()[0] if lang else ""
    name = LANG_ALIAS.get(lang, lang)
    if name:
        try:
            lexer = get_lexer_by_name(name)
            return highlight(code, lexer, HtmlFormatter(nowrap=True))
        except ClassNotFound:
            pass
    return ""  # 空字串 → markdown-it 自己 escape


def make_md():
    md = MarkdownIt("commonmark", {"html": False, "highlight": highlight_code, "typographer": False})
    md.enable(["table", "strikethrough"])
    return md


def rewrite_href(href, src_rel, converted, out_dir=None):
    """把 .md 原稿裡的相對連結改成 html/ 底下可用的路徑；out_dir 是產生的 HTML 所在資料夾。"""
    if not href or href.startswith(("#", "http://", "https://", "mailto:")) or "://" in href:
        return href
    path, _, frag = href.partition("#")
    target = (ROOT / src_rel.parent / unquote(path)).resolve()
    try:
        target_rel = target.relative_to(ROOT)
    except ValueError:
        return href
    self_page = out_dir is None
    out_dir = out_dir or OUT / src_rel.parent
    if target.is_dir() and (target_rel / "README.md").as_posix() in converted:
        target_rel = target_rel / "README.md"  # 連到資料夾：改連它的 README 頁，不要落到伺服器的目錄清單
        target = ROOT / target_rel
    if self_page and target == (OUT / src_rel.with_suffix(".html")).resolve() and frag:
        return "#" + frag  # 原稿連到自己的 HTML 版：同頁錨點
    if target_rel.as_posix() in converted:
        new = os.path.relpath(OUT / target_rel.with_suffix(".html"), out_dir)
    else:
        # 沒轉的檔（small-project、review、追蹤檔、原始碼）：直接指回 repo 裡的原檔
        new = os.path.relpath(target, out_dir)
        if path.endswith("/") and not new.endswith("/"):
            new += "/"
    new = quote(new.replace(os.sep, "/"))
    return new + ("#" + frag if frag else "")


CJK = re.compile(r"[\u2e80-\u9fff\uf900-\ufaff\uff00-\uffef\u3000-\u303f]")
META = re.compile(r"^[^\s：]{1,12}：")


def fix_softbreaks(token, in_quote):
    """中文斷行接起來不要多一個空白；引言區塊裡「欄位：」開頭的行（metadata）各自換行。"""
    ch = token.children or []
    for j, c in enumerate(ch):
        if c.type != "softbreak":
            continue
        prev = ch[j - 1].content if j > 0 else ""
        nxt = ch[j + 1].content if j + 1 < len(ch) else ""
        if in_quote and META.match(nxt):
            c.type = "hardbreak"
        elif prev and nxt and CJK.match(prev[-1]) and CJK.match(nxt[0]):
            c.type, c.content = "text", ""


STRONG = re.compile(r"\*\*(?=[^\s*])([^\n]*?[^\s*])\*\*")
CODE_SPAN = re.compile(r"<pre[\s>].*?</pre>|<code>.*?</code>", re.S)


def fix_cjk_strong(body):
    """CommonMark 的 flanking 規則讓 `**結論：**中文` 這種寫法變成字面上的 **，這裡補成粗體。"""
    saved = []

    def stash(m):
        saved.append(m.group(0))
        return f"\x00{len(saved) - 1}\x00"

    body = STRONG.sub(r"<strong>\1</strong>", CODE_SPAN.sub(stash, body))
    return re.sub(r"\x00(\d+)\x00", lambda m: saved[int(m.group(1))], body)


def fix_nested_strong(token, md):
    """整段 `**一句話結論：…**重點**…**` 裡又包粗體時，CommonMark 會配錯對；改成外層整段粗體、內層保留。"""
    c = token.content
    if not (c.startswith("**") and c.endswith("**") and c.count("**") > 2 and c.count("**") % 2 == 0):
        return
    if not any(ch.type == "text" and "**" in ch.content for ch in token.children or []):
        return  # 原本就配對正確
    inner = md.parseInline(c[2:-2], {})[0].children
    token.children = [Token("strong_open", "strong", 1, markup="**")] + inner + [
        Token("strong_close", "strong", -1, markup="**")]


# 2026-10-08 以前的作答框是舊格式（批改不一定緊接在下面），一律當歷程、不給編輯
EDITABLE_SINCE = "2026-10-08"
DATE = re.compile(r"\d{4}-\d{2}-\d{2}")
ANSWER_PREV = re.compile(r"^(\*\*.*(次|答).*\*\*|> 盒子|> 過去的作答|> 題目：|#{3,4} )")


def mark_answer_boxes(tokens, src_rel, lines):
    """作答區（small-project、review）裡沒標語言的程式碼區塊，前一行是作答框標籤、而且下面還沒有批改的，換成輸入框。"""
    if src_rel.parts[0] not in PRACTICE_DIRS:
        return 0
    n = 0
    for t in tokens:
        if t.type != "fence" or t.info.strip() or not t.map:
            continue
        start, end = t.map
        k = start - 1
        while k >= 0 and not lines[k].strip():
            k -= 1
        if k < 0 or not ANSWER_PREV.match(lines[k].strip()):
            continue
        # 日期在作答標籤上；標籤和作答框中間夾了 `> 題目：` 時往上多看一行；都沒有就用複習檔檔名的日期
        label = lines[k]
        if label.startswith("> 題目：") and k > 0:
            label = lines[k - 2] if not lines[k - 1].strip() and k > 1 else lines[k - 1]
        d = DATE.search(label) or DATE.match(src_rel.name)
        if d and d.group(0) < EDITABLE_SINCE:
            continue
        k = end
        while k < len(lines) and not lines[k].strip():
            k += 1
        graded = k < len(lines) and lines[k].strip().startswith("> **批改")
        if graded:
            continue
        t.type = "html_block"
        t.content = (f'<div class="answer-box" data-file="{html.escape(src_rel.as_posix())}" data-line="{start}">'
                     f'<textarea spellcheck="false" placeholder="在這裡作答">{html.escape(t.content)}</textarea>'
                     '<div class="answer-bar"><button class="save" onclick="saveAnswer(this)">儲存</button>'
                     '<span class="status"></span></div></div>\n')
        n += 1
    return n


def transform(tokens, src_rel, converted, md, out_dir=None):
    """改連結、加 heading id、把「參考答案」引言區塊換成 <details>。回傳 (title, toc)。"""
    title, toc, seen = None, [], {}
    stack = []  # 每個 blockquote_open 是否被換成 details
    for i, t in enumerate(tokens):
        if t.type == "heading_open":
            text = inline_text(tokens[i + 1])
            base = slugify(text)
            n = seen.get(base, 0)
            seen[base] = n + 1
            hid = base if n == 0 else f"{base}-{n}"
            t.attrSet("id", hid)
            level = int(t.tag[1])
            if level == 1 and title is None:
                title = text
            elif level in (2, 3):
                toc.append((level, text, hid))
        elif t.type == "inline":
            fix_nested_strong(t, md)
            fix_softbreaks(t, in_quote=bool(stack))
            ch = t.children or []
            for j, c in enumerate(ch):
                if c.type == "link_open":
                    c.attrSet("href", rewrite_href(c.attrGet("href"), src_rel, converted, out_dir))
                    # 連到有 HTML 版的筆記時，連結文字的檔名不要再顯示 .md
                    nxt = ch[j + 1] if j + 1 < len(ch) else None
                    if (c.attrGet("href").partition("#")[0].endswith(".html") and nxt
                            and nxt.type in ("text", "code_inline") and nxt.content.endswith(".md")):
                        nxt.content = nxt.content[:-3]
                elif c.type == "image":
                    c.attrSet("src", rewrite_href(c.attrGet("src"), src_rel, converted, out_dir))
        elif t.type == "blockquote_open":
            is_answer = False
            if i + 2 < len(tokens) and tokens[i + 1].type == "paragraph_open":
                ch = tokens[i + 2].children or []
                k = 1 if ch and ch[0].type == "text" and ch[0].content == "" else 0
                if (len(ch) >= k + 3 and ch[k].type == "strong_open"
                        and ch[k + 1].content.strip() == "參考答案"):
                    is_answer = True
                    del ch[0:k + 3]
            stack.append(is_answer)
            if is_answer:
                t.type, t.tag, t.nesting = "html_block", "", 0
                t.content = '<details class="answer"><summary>參考答案</summary>\n'
        elif t.type == "blockquote_close":
            if stack.pop():
                t.type, t.tag, t.nesting = "html_block", "", 0
                t.content = "</details>\n"
    return title, toc


# ---------- 版面 ----------

CSS = """
:root{--bg:#fdfdfc;--fg:#1f2328;--muted:#656d76;--line:#d8dee4;--code-bg:#f3f4f6;--link:#0969da;
--side:#f6f8fa;--accent:#bf8700;--ans:#fff8e1;--q:#eef6ff}
@media (prefers-color-scheme:dark){:root:not([data-theme="light"]){--bg:#0e1116;--fg:#e6edf3;--muted:#9198a1;
--line:#30363d;--code-bg:#161b22;--link:#4493f8;--side:#11151b;--accent:#d29922;--ans:#2a2414;--q:#132033}}
:root[data-theme="dark"]{--bg:#0e1116;--fg:#e6edf3;--muted:#9198a1;--line:#30363d;--code-bg:#161b22;
--link:#4493f8;--side:#11151b;--accent:#d29922;--ans:#2a2414;--q:#132033}
*{box-sizing:border-box}
html{scroll-padding-top:16px}
body{margin:0;background:var(--bg);color:var(--fg);font:16px/1.75 -apple-system,"Segoe UI","Noto Sans TC",
"PingFang TC","Microsoft JhengHei",sans-serif}
a{color:var(--link);text-decoration:none}a:hover{text-decoration:underline}
.layout{display:flex;align-items:flex-start}
nav.side{position:sticky;top:0;width:290px;flex-shrink:0;height:100vh;overflow-y:auto;background:var(--side);
border-right:1px solid var(--line);padding:16px 14px;font-size:14px;line-height:1.5}
nav.side .crumbs{margin-bottom:10px}
nav.side .tools{display:flex;gap:6px;flex-wrap:wrap;margin-bottom:14px}
nav.side ul{list-style:none;padding-left:0;margin:0}
nav.side li{margin:3px 0}nav.side li.l3{padding-left:14px;font-size:13px}
nav.side li a{color:var(--fg);display:block;padding:2px 6px;border-radius:4px}
nav.side li a:hover{background:var(--line);text-decoration:none}
nav.side li a.active{background:var(--line);font-weight:600}
button{font:inherit;font-size:13px;background:var(--bg);color:var(--fg);border:1px solid var(--line);
border-radius:6px;padding:3px 9px;cursor:pointer}button:hover{border-color:var(--muted)}
main{flex:1;min-width:0;max-width:960px;padding:24px 40px 80px;margin:0 auto}
h1{font-size:1.9em;line-height:1.3;border-bottom:1px solid var(--line);padding-bottom:.3em}
h2{font-size:1.45em;border-bottom:1px solid var(--line);padding-bottom:.25em;margin-top:2em}
h3{font-size:1.2em;margin-top:1.6em}h4{font-size:1.05em}
h1,h2,h3,h4{position:relative}
.anchor{opacity:0;margin-left:6px;font-size:.8em;color:var(--muted)}h1:hover .anchor,h2:hover .anchor,
h3:hover .anchor,h4:hover .anchor{opacity:1}
code{font-family:"JetBrains Mono","Fira Code",ui-monospace,Menlo,Consolas,monospace;font-size:.88em;
background:var(--code-bg);padding:.15em .35em;border-radius:4px}
pre{background:var(--code-bg);border:1px solid var(--line);border-radius:6px;padding:12px 14px;overflow-x:auto;
line-height:1.5}pre code{background:none;padding:0;font-size:.85em}
blockquote{margin:1em 0;padding:.3em 1em;border-left:4px solid var(--line);color:var(--muted)}
blockquote p{margin:.4em 0}
table{border-collapse:collapse;display:block;overflow-x:auto;margin:1em 0;max-width:100%}
th,td{border:1px solid var(--line);padding:6px 10px;vertical-align:top}th{background:var(--code-bg)}
hr{border:0;border-top:1px solid var(--line);margin:2em 0}
img{max-width:100%}
details.answer{margin:.6em 0;border:1px solid var(--accent);border-radius:6px;background:var(--ans);padding:0 12px}
details.answer>summary{cursor:pointer;color:var(--accent);font-weight:600;padding:6px 0;user-select:none}
details.answer[open]>summary{border-bottom:1px dashed var(--accent);margin-bottom:6px}
.menu-btn{display:none}
.answer-box{margin:.6em 0 1em;border:1px solid var(--link);border-radius:6px;background:var(--q);padding:8px}
.answer-box.dirty{border-color:var(--accent)}
.answer-box textarea{width:100%;min-height:5.5em;resize:vertical;border:1px solid var(--line);border-radius:4px;
padding:8px 10px;background:var(--bg);color:var(--fg);font:14px/1.55 "JetBrains Mono","Fira Code",ui-monospace,
Menlo,Consolas,"Noto Sans Mono CJK TC",monospace}
.answer-bar{display:flex;align-items:center;gap:10px;margin-top:6px}
.answer-bar .status{font-size:13px;color:var(--muted)}.answer-bar .status.err{color:#cf222e}
.answer-box.dirty .status{color:var(--accent)}
.practice{display:flex;gap:10px;flex-wrap:wrap;margin:0 0 6px}
.practice a{border:1px solid var(--line);border-radius:6px;padding:6px 12px}
@media (max-width:900px){.layout{display:block}nav.side{position:fixed;z-index:10;left:0;top:0;
transform:translateX(-100%);transition:transform .2s;box-shadow:2px 0 12px rgba(0,0,0,.25)}
nav.side.open{transform:none}main{padding:56px 16px 60px}
.menu-btn{display:block;position:fixed;top:10px;left:10px;z-index:11}}
/* index */
.index table{display:table;width:100%;table-layout:fixed;font-size:14px;line-height:1.6}
.index th:nth-child(1){width:26%}.index th:nth-child(2){width:50%}.index th:nth-child(3){width:24%}
.index td:nth-child(3) code{font-size:.8em;display:inline-block;margin:1px 0}
.index td{word-break:break-word}.index h3{margin-top:1.4em}.index .intro{color:var(--muted)}
.empty{color:var(--muted)}
@media (max-width:700px){.index table{display:block;table-layout:auto}.index th:nth-child(3),.index td:nth-child(3){display:none}}
.search{width:100%;font:inherit;padding:10px 14px;border:1px solid var(--line);border-radius:8px;
background:var(--bg);color:var(--fg);margin:8px 0 20px}
.domain{margin-bottom:28px}.domain h2{margin-top:0}
.count{color:var(--muted);font-size:14px;font-weight:normal}
"""

def pyg_css():
    # pygments 的 class（.k、.s…）掛在 pre code 底下，依主題切換兩套配色
    light = HtmlFormatter(style="default").get_style_defs("pre code")
    dark = HtmlFormatter(style="github-dark").get_style_defs("pre code")
    strip_bg = lambda s: re.sub(r"pre code \{[^}]*\}", "", re.sub(r"pre code \.hll[^}]*\}", "", s))
    light, dark = strip_bg(light), strip_bg(dark)
    dark_scoped = lambda sel: "\n".join(
        sel + " " + line for line in dark.splitlines() if line.startswith("pre code"))
    # 程式碼裡夾中文註解時 lexer 會標成 .err（紅框），不要顯示
    light += "\npre code .err{border:0;color:inherit;background:none}"
    return (light + "\n@media (prefers-color-scheme:dark){\n"
            + dark_scoped(':root:not([data-theme="light"])') + "\n}\n"
            + dark_scoped(':root[data-theme="dark"]') + "\n")


JS = """
(function(){
var root=document.documentElement;
try{var t=localStorage.getItem('theme');if(t)root.setAttribute('data-theme',t);}catch(e){}
window.toggleTheme=function(){var dark=root.getAttribute('data-theme')==='dark'||
(!root.getAttribute('data-theme')&&matchMedia('(prefers-color-scheme: dark)').matches);
var n=dark?'light':'dark';root.setAttribute('data-theme',n);try{localStorage.setItem('theme',n);}catch(e){}};
window.toggleAnswers=function(open){document.querySelectorAll('details.answer').forEach(function(d){d.open=open;});};
window.toggleMenu=function(){document.querySelector('nav.side').classList.toggle('open');};
// 作答框：按「儲存」透過 tools/serve.py 寫回 .md 原稿
function fit(ta){ta.style.height='auto';ta.style.height=(ta.scrollHeight+2)+'px';}
window.saveAnswer=function(btn){
  var box=btn.closest('.answer-box'),ta=box.querySelector('textarea'),st=box.querySelector('.status');
  if(location.protocol==='file:'){st.textContent='要存檔請用 python3 tools/serve.py 開啟';return;}
  btn.disabled=true;st.className='status';st.textContent='儲存中…';
  fetch('/api/save',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({file:box.dataset.file,line:+box.dataset.line,original:ta.defaultValue,content:ta.value})})
  .then(function(r){return r.json().then(function(j){return {ok:r.ok,j:j};});})
  .then(function(x){
    btn.disabled=false;
    if(!x.ok){st.className='status err';st.textContent='✗ '+x.j.error;return;}
    var line=+box.dataset.line;
    document.querySelectorAll('.answer-box').forEach(function(b){
      if(b!==box&&b.dataset.file===box.dataset.file&&+b.dataset.line>line)b.dataset.line=+b.dataset.line+x.j.delta;});
    ta.defaultValue=ta.value;box.classList.remove('dirty');
    st.textContent='✓ 已儲存 '+new Date().toLocaleTimeString();})
  .catch(function(e){btn.disabled=false;st.className='status err';st.textContent='✗ 連不到 serve.py：'+e;});
};
window.addEventListener('beforeunload',function(e){
  if(document.querySelector('.answer-box.dirty')){e.preventDefault();e.returnValue='';}});
document.addEventListener('DOMContentLoaded',function(){
  document.querySelectorAll('main h1[id],main h2[id],main h3[id],main h4[id]').forEach(function(h){
    var a=document.createElement('a');a.className='anchor';a.href='#'+h.id;a.textContent='#';h.appendChild(a);});
  document.querySelectorAll('.answer-box textarea').forEach(function(ta){
    var box=ta.closest('.answer-box'),st=box.querySelector('.status');fit(ta);
    if(location.protocol==='file:'){ta.readOnly=true;box.querySelector('.save').disabled=true;
      st.textContent='唯讀：用 python3 tools/serve.py 開啟才能作答存檔';return;}
    ta.addEventListener('input',function(){fit(ta);var d=ta.value!==ta.defaultValue;box.classList.toggle('dirty',d);
      st.className='status';st.textContent=d?'未儲存':'';});
    ta.addEventListener('keydown',function(e){if((e.ctrlKey||e.metaKey)&&e.key==='s'){e.preventDefault();
      saveAnswer(box.querySelector('.save'));}});});
  var links=[].slice.call(document.querySelectorAll('nav.side li a'));
  if(!links.length||!('IntersectionObserver' in window))return;
  var map={};links.forEach(function(a){map[decodeURIComponent(a.hash.slice(1))]=a;});
  var obs=new IntersectionObserver(function(es){es.forEach(function(e){if(e.isIntersecting&&map[e.target.id]){
    links.forEach(function(a){a.classList.remove('active');});map[e.target.id].classList.add('active');}});},
    {rootMargin:'0px 0px -75% 0px'});
  document.querySelectorAll('main h2[id],main h3[id]').forEach(function(h){obs.observe(h);});
});
})();
"""


def page(title, side, body, depth):
    css_href = "../" * depth + "assets/style.css"
    js_href = "../" * depth + "assets/app.js"
    return f"""<!doctype html>
<html lang="zh-Hant">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>{html.escape(title)}</title>
<link rel="stylesheet" href="{css_href}">
<script src="{js_href}"></script>
</head>
<body>
<button class="menu-btn" onclick="toggleMenu()">☰ 目錄</button>
<div class="layout">
<nav class="side">
{side}
</nav>
<main>
{body}
</main>
</div>
</body>
</html>
"""


def side_nav(src_rel, toc, has_answers):
    """側欄：麵包屑、工具按鈕、本頁目錄。"""
    depth = len(src_rel.parts) - 1
    home = "../" * depth + "index.html"
    crumbs = f'<a href="{home}">🏠 首頁</a>'
    # 每一層有 README 的資料夾都列一個麵包屑（small-project/s1-05-asm-calls/ 會有兩層）
    for i in range(1, len(src_rel.parts)):
        folder = Path(*src_rel.parts[:i])
        name = html.escape(src_rel.parts[i - 1])
        if (ROOT / folder / "README.md").exists() and src_rel != folder / "README.md":
            href = "../" * (depth - i) + "README.html"
            crumbs += f' / <a href="{href}">{name}</a>'
        else:
            crumbs += f" / {name}"
    tools = '<button onclick="toggleTheme()">🌓 主題</button>'
    if has_answers:
        tools += ('<button onclick="toggleAnswers(true)">展開答案</button>'
                  '<button onclick="toggleAnswers(false)">收起答案</button>')
    items = "\n".join(
        f'<li class="l{lv}"><a href="#{quote(hid)}">{html.escape(text)}</a></li>' for lv, text, hid in toc)
    return f'<div class="crumbs">{crumbs}</div>\n<div class="tools">{tools}</div>\n<ul>\n{items}\n</ul>'


# ---------- 首頁 ----------

def readme_parts(readme_rel):
    """拆出領域 README 的 (標題, 開頭說明, 「## 筆記列表」那一節)；分類導覽清單與結尾提醒不要。"""
    lines = (ROOT / readme_rel).read_text(encoding="utf-8").splitlines()
    title = lines[0].lstrip("# ").strip()
    intro, section, state = [], [], "intro"
    for line in lines[1:]:
        if line.startswith("## "):
            if state == "list":
                break
            state = "list" if line.strip() == "## 筆記列表" else "other"
            continue
        if state == "intro":
            intro.append(line)
        elif state == "list":
            if re.match(r"^- \[[^\]]+\]\(#", line) or line.startswith("> 新增筆記") or line.strip() == "---":
                continue
            section.append(line)
    return title, "\n".join(intro).strip(), "\n".join(section)


def render_readme_part(text, readme_rel, dom, converted, md):
    """用跟筆記一樣的規則轉 README 片段，但連結以首頁（html/）為基準、id 加上領域前綴避免撞名。"""
    tokens = md.parse(text)
    _, toc = transform(tokens, readme_rel, converted, md, out_dir=OUT)
    for t in tokens:
        if t.type == "heading_open":
            t.attrSet("id", f"{dom}-{t.attrGet('id')}")
    body = fix_cjk_strong(md.renderer.render(tokens, md.options, {}))
    return body, [(lv, text, f"{dom}-{hid}") for lv, text, hid in toc]


def build_index(domains, converted, md):
    blocks, side_items, total = [], [], 0
    for dom in domains:
        readme = Path(dom) / "README.md"
        title, intro, section = readme_parts(readme)
        intro_html, _ = render_readme_part(intro, readme, dom, converted, md)
        list_html, toc = render_readme_part(section, readme, dom, converted, md)
        # 每個分類（### 開頭到下一個 ###）包成一個區塊，搜尋時整塊沒命中就藏起來
        pieces = re.split(r"(?=<h3 )", list_html)
        list_html = "".join(f'<div class="cat">{x}</div>' for x in pieces if x.strip())
        n = sum(1 for r in converted if r.startswith(dom + "/") and not r.endswith("README.md"))
        total += n
        blocks.append(
            f'<section class="domain"><h2 id="{html.escape(dom)}">{html.escape(title)} '
            f'<span class="count">{n} 篇 · <a href="{quote(dom)}/README.html">完整 README</a></span></h2>\n'
            f'<div class="intro">{intro_html}</div>\n{list_html}</section>')
        side_items.append(f'<li class="l2"><a href="#{quote(dom)}">{html.escape(title.replace(" 筆記索引", ""))}'
                          f'</a></li>')
        side_items += [f'<li class="l3"><a href="#{quote(hid)}">{html.escape(text)}</a></li>'
                       for lv, text, hid in toc if lv == 3]
    side = ('<div class="crumbs">🏠 首頁</div><div class="tools"><button onclick="toggleTheme()">🌓 主題</button></div>'
            "<ul>" + "".join(side_items) + "</ul>")
    reviews = sorted((ROOT / "review").glob("20*.md"))
    practice = ('<div class="practice"><a href="small-project/README.html">✍️ 課程練習區（small-project）</a>'
                '<a href="review/README.html">🔁 間隔複習（review）</a>'
                + (f'<a href="review/{quote(reviews[-1].stem)}.html">📅 最新複習：{html.escape(reviews[-1].stem)}</a>'
                   if reviews else "") + "</div>")
    body = ('<h1>筆記總索引</h1>' + practice +
            f'<input class="search" id="q" placeholder="搜尋筆記、主題、關鍵字（共 {total} 篇，空白分隔多個詞）" autofocus>'
            '<p class="empty" id="empty" hidden>沒有符合的筆記。</p>'
            + '<div class="index">' + "\n".join(blocks) + "</div>"
            + "<script>(function(){var q=document.getElementById('q');"
            "var rows=[].slice.call(document.querySelectorAll('.index tbody tr'));"
            "rows.forEach(function(r){r._t=r.textContent.toLowerCase();});"
            "q.addEventListener('input',function(){"
            "var ws=q.value.toLowerCase().split(/\\s+/).filter(Boolean),hits=0;"
            "rows.forEach(function(r){var ok=ws.every(function(w){return r._t.indexOf(w)>=0;});"
            "r.style.display=ok?'':'none';if(ok)hits++;});"
            "var vis=function(el){return [].some.call(el.querySelectorAll('tbody tr'),function(r){return r.style.display!=='none';});};"
            "document.querySelectorAll('.index .cat').forEach(function(c){"
            "c.style.display=(!ws.length||vis(c))?'':'none';});"
            "document.querySelectorAll('.index .domain').forEach(function(d){"
            "d.style.display=(!ws.length||vis(d))?'':'none';"
            "var i=d.querySelector('.intro');if(i)i.style.display=ws.length?'none':'';});"
            "document.getElementById('empty').hidden=!(ws.length&&!hits);});})();</script>")
    (OUT / "index.html").write_text(page("筆記總索引", side, body, 0), encoding="utf-8")


def main():
    files = note_files()
    converted = {f.as_posix() for f in files}
    md = make_md()
    if OUT.exists():
        shutil.rmtree(OUT)
    (OUT / "assets").mkdir(parents=True)
    (OUT / "assets" / "style.css").write_text(CSS + pyg_css(), encoding="utf-8")
    (OUT / "assets" / "app.js").write_text(JS, encoding="utf-8")

    titles, domains = {}, []
    for rel in files:
        src = (ROOT / rel).read_text(encoding="utf-8")
        tokens = md.parse(src)
        mark_answer_boxes(tokens, rel, src.splitlines())
        title, toc = transform(tokens, rel, converted, md)
        title = title or rel.stem
        titles[rel.as_posix()] = title
        if rel.name == "README.md" and rel.parts[0] not in domains and rel.parts[0] not in PRACTICE_DIRS:
            domains.append(rel.parts[0])
        body = fix_cjk_strong(md.renderer.render(tokens, md.options, {}))
        has_answers = 'class="answer"' in body
        out = OUT / rel.with_suffix(".html")
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(page(title, side_nav(rel, toc, has_answers), body, len(rel.parts) - 1), encoding="utf-8")

    missing = sorted({f.parts[0] for f in files} - set(domains) - set(PRACTICE_DIRS))
    if missing:
        print("警告：這些領域沒有 README.md：", missing, file=sys.stderr)
    build_index(sorted(domains), converted, md)
    print(f"已產生 {len(files)} 篇 → {OUT.relative_to(ROOT)}/（首頁：html/index.html）")


if __name__ == "__main__":
    main()
