
(function(){
var root=document.documentElement;
try{var t=localStorage.getItem('theme');if(t)root.setAttribute('data-theme',t);}catch(e){}
window.toggleTheme=function(){var dark=root.getAttribute('data-theme')==='dark'||
(!root.getAttribute('data-theme')&&matchMedia('(prefers-color-scheme: dark)').matches);
var n=dark?'light':'dark';root.setAttribute('data-theme',n);try{localStorage.setItem('theme',n);}catch(e){}
renderMermaid(true);};
// ```mermaid 的圖：照目前主題畫；收起的 <details> 裡量不到大小，打開時才畫；換主題時重畫
function isDark(){var t=root.getAttribute('data-theme');return t?t==='dark':matchMedia('(prefers-color-scheme: dark)').matches;}
function renderMermaid(redo){
  if(!window.mermaid)return;
  mermaid.initialize({startOnLoad:false,securityLevel:'strict',theme:isDark()?'dark':'default'});
  var nodes=[].slice.call(document.querySelectorAll('pre.mermaid')).filter(function(p){
    if(p.dataset.src===undefined)p.dataset.src=p.textContent;
    if(redo&&p.getAttribute('data-processed')){p.removeAttribute('data-processed');p.textContent=p.dataset.src;}
    if(p.getAttribute('data-processed'))return false;
    for(var d=p.closest('details');d;d=d.parentElement.closest('details'))if(!d.open)return false;
    return true;});
  if(nodes.length)mermaid.run({nodes:nodes});
}
window.toggleAnswers=function(open){document.querySelectorAll('details.answer').forEach(function(d){d.open=open;});};
window.toggleBoxes=function(open){document.querySelectorAll('details.answer-fold').forEach(function(d){d.open=open;});};
window.toggleMenu=function(){document.querySelector('nav.side').classList.toggle('open');};
// 作答框：按「儲存」透過 tools/serve.py 寫回 .md 原稿
function fit(ta){ta.style.height='auto';ta.style.height=(ta.scrollHeight+2)+'px';}
window.saveAnswer=function(btn){
  var box=btn.closest('.answer-box'),ta=box.querySelector('textarea'),st=box.querySelector('.status');
  if(location.protocol==='file:'){st.textContent='要存檔請用 python3 tools/serve.py 開啟';return;}
  btn.disabled=true;st.className='status';st.textContent='儲存中…';
  fetch('/api/save',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({file:box.dataset.file,line:+box.dataset.line,original:ta.defaultValue,content:ta.value,
      insert:box.dataset.insert==='1',anchor:box.dataset.anchor})})
  .then(function(r){return r.json().then(function(j){return {ok:r.ok,j:j};});})
  .then(function(x){
    btn.disabled=false;
    if(!x.ok){st.className='status err';st.textContent='✗ '+x.j.error;return;}
    var line=+box.dataset.line;
    document.querySelectorAll('.answer-box').forEach(function(b){
      if(b!==box&&b.dataset.file===box.dataset.file&&+b.dataset.line>line)b.dataset.line=+b.dataset.line+x.j.delta;});
    if(x.j.line!==undefined){box.dataset.line=x.j.line;delete box.dataset.insert;delete box.dataset.anchor;}
    ta.defaultValue=ta.value;box.classList.remove('dirty');
    st.textContent='✓ 已儲存 '+new Date().toLocaleTimeString();})
  .catch(function(e){btn.disabled=false;st.className='status err';st.textContent='✗ 連不到 serve.py：'+e;});
};
window.addEventListener('beforeunload',function(e){
  if(document.querySelector('.answer-box.dirty')){e.preventDefault();e.returnValue='';}});
// 程式碼區塊的「複製」按鈕；navigator.clipboard 只在 localhost／https 能用，file:// 改用 execCommand
function copyText(text){
  if(navigator.clipboard&&window.isSecureContext)return navigator.clipboard.writeText(text);
  return new Promise(function(ok,fail){var ta=document.createElement('textarea');ta.value=text;
    ta.style.position='fixed';ta.style.opacity='0';document.body.appendChild(ta);ta.select();
    var r=false;try{r=document.execCommand('copy');}catch(e){}document.body.removeChild(ta);r?ok():fail();});
}
function addCopyButtons(){
  document.querySelectorAll('main pre:not(.mermaid)').forEach(function(pre){
    var wrap=document.createElement('div');wrap.className='code-wrap';
    pre.parentNode.insertBefore(wrap,pre);wrap.appendChild(pre);
    var b=document.createElement('button');b.className='copy-btn';b.type='button';b.textContent='複製';
    b.onclick=function(){var code=pre.querySelector('code')||pre;
      copyText(code.textContent).then(function(){b.textContent='✓ 已複製';b.classList.add('done');},
        function(){b.textContent='複製失敗';})
      .then(function(){setTimeout(function(){b.textContent='複製';b.classList.remove('done');},1500);});};
    wrap.appendChild(b);});
}
document.addEventListener('DOMContentLoaded',function(){
  addCopyButtons();
  renderMermaid(false);
  document.querySelectorAll('details').forEach(function(d){d.addEventListener('toggle',function(){if(d.open)renderMermaid(false);});});
  // 收起時 textarea 量不到高度，打開自我練習框、作答框時再算一次
  document.querySelectorAll('details.self-practice,details.answer-fold').forEach(function(d){d.addEventListener('toggle',function(){
    if(d.open)d.querySelectorAll('textarea').forEach(fit);});});
  document.querySelectorAll('main h1[id],main h2[id],main h3[id],main h4[id]').forEach(function(h){
    var a=document.createElement('a');a.className='anchor';a.href='#'+h.id;a.textContent='#';h.appendChild(a);});
  document.querySelectorAll('.answer-box textarea').forEach(function(ta){
    var box=ta.closest('.answer-box'),st=box.querySelector('.status');fit(ta);
    if(box.classList.contains('locked'))return;
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
