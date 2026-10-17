
(function(){
var root=document.documentElement;
try{var t=localStorage.getItem('theme');if(t)root.setAttribute('data-theme',t);}catch(e){}
window.toggleTheme=function(){var dark=root.getAttribute('data-theme')==='dark'||
(!root.getAttribute('data-theme')&&matchMedia('(prefers-color-scheme: dark)').matches);
var n=dark?'light':'dark';root.setAttribute('data-theme',n);try{localStorage.setItem('theme',n);}catch(e){}};
window.toggleAnswers=function(open){document.querySelectorAll('details.answer').forEach(function(d){d.open=open;});};
window.toggleMenu=function(){document.querySelector('nav.side').classList.toggle('open');};
document.addEventListener('DOMContentLoaded',function(){
  document.querySelectorAll('main h1[id],main h2[id],main h3[id],main h4[id]').forEach(function(h){
    var a=document.createElement('a');a.className='anchor';a.href='#'+h.id;a.textContent='#';h.appendChild(a);});
  var links=[].slice.call(document.querySelectorAll('nav.side li a'));
  if(!links.length||!('IntersectionObserver' in window))return;
  var map={};links.forEach(function(a){map[decodeURIComponent(a.hash.slice(1))]=a;});
  var obs=new IntersectionObserver(function(es){es.forEach(function(e){if(e.isIntersecting&&map[e.target.id]){
    links.forEach(function(a){a.classList.remove('active');});map[e.target.id].classList.add('active');}});},
    {rootMargin:'0px 0px -75% 0px'});
  document.querySelectorAll('main h2[id],main h3[id]').forEach(function(h){obs.observe(h);});
});
})();
