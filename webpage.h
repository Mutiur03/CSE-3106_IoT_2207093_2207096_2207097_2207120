// ============================================================================
//  webpage.h  -  Single-page UI served from ESP32 flash (PROGMEM).
//  Joint-jog only. Talks to firmware over WebSocket with JSON messages.
// ============================================================================
#pragma once

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html><html lang="en"><head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>RoboArm Control</title>
<style>
  :root{--bg:#0f1115;--card:#1a1e26;--fg:#e6e9ef;--mut:#8b93a3;--acc:#4da3ff;--ok:#39d353;--err:#ff5d5d}
  *{box-sizing:border-box}
  body{margin:0;font:15px/1.4 system-ui,sans-serif;background:var(--bg);color:var(--fg)}
  header{padding:14px 18px;display:flex;align-items:center;gap:10px;border-bottom:1px solid #262b36}
  h1{font-size:17px;margin:0;font-weight:600}
  #dot{width:10px;height:10px;border-radius:50%;background:var(--err)}
  #dot.on{background:var(--ok)}
  main{max-width:560px;margin:0 auto;padding:16px;display:grid;gap:16px}
  .card{background:var(--card);border:1px solid #262b36;border-radius:12px;padding:16px}
  .card h2{margin:0 0 12px;font-size:13px;letter-spacing:.04em;text-transform:uppercase;color:var(--mut)}
  button{background:var(--acc);color:#04121f;border:0;border-radius:8px;padding:10px 14px;
    font-size:14px;font-weight:600;cursor:pointer;user-select:none;-webkit-user-select:none;touch-action:none}
  button.ghost{background:#232935;color:var(--fg)}
  button:active{transform:translateY(1px)}
  .jog{display:grid;grid-template-columns:auto 1fr auto;gap:8px;align-items:center;margin-bottom:8px}
  .jog span{font-variant-numeric:tabular-nums}
  .readout{display:grid;grid-template-columns:repeat(2,1fr);gap:8px;font-variant-numeric:tabular-nums}
  .readout div{background:#0f1319;border-radius:8px;padding:8px 10px}
  .readout b{color:var(--acc)}
  #msg{min-height:18px;font-size:13px;color:var(--mut)}
  #msg.err{color:var(--err)} #msg.ok{color:var(--ok)}
  .row{display:flex;gap:10px;flex-wrap:wrap;align-items:center;justify-content:center}
</style></head><body>
<header><span id="dot"></span><h1>Robotic Arm</h1><span id="conn" style="color:var(--mut);font-size:12px">connecting...</span></header>
<main>

  <div class="card">
    <h2>Joint jog (hold to move)</h2>
    <div id="jogs"></div>
    <div class="row" style="margin-top:10px"><button class="ghost" onclick="send({cmd:'stop'})">Stop</button></div>
    <p id="msg"></p>
  </div>

  <div class="card">
    <h2>State</h2>
    <div class="readout">
      <div>J1 base <b id="rj1">-</b>&deg;</div>
      <div>J2 shoulder <b id="rj2">-</b>&deg;</div>
      <div>J3 elbow <b id="rj3">-</b>&deg;</div>
      <div>J4 wrist <b id="rj4">-</b>&deg;</div>
    </div>
    <div class="row" style="margin-top:12px">
      <button class="ghost" onclick="send({cmd:'sethome'})">Set home</button>
    </div>
    <p style="color:var(--mut);font-size:12px;margin:8px 0 0">
      J1 open-loop (continuous servo). Re-home when drift builds.
    </p>
  </div>
</main>
<script>
const $=id=>document.getElementById(id);
let ws;
function connect(){
  ws=new WebSocket('ws://'+location.host+'/ws');
  ws.onopen =()=>{ $('dot').classList.add('on'); $('conn').textContent='connected'; };
  ws.onclose=()=>{ $('dot').classList.remove('on'); $('conn').textContent='reconnecting...'; setTimeout(connect,1000); };
  ws.onmessage=e=>{ try{update(JSON.parse(e.data));}catch(_){} };
}
function send(o){ if(ws&&ws.readyState==1) ws.send(JSON.stringify(o)); }
function update(s){
  if(s.j){ $('rj1').textContent=s.j[0].toFixed(1); $('rj2').textContent=s.j[1].toFixed(1);
           $('rj3').textContent=s.j[2].toFixed(1); $('rj4').textContent=s.j[3].toFixed(1); }
  if(s.msg!==undefined){ const m=$('msg'); m.textContent=s.msg; m.className=s.ok===false?'err':(s.msg?'ok':''); }
}
function jog(j,d){ send({cmd:'jog',joint:j,delta:d}); }
const JOINTS=[['J1 base',1],['J2 shoulder',2],['J3 elbow',3],['J4 wrist',4]];
const STEP={1:5, 2:3, 3:3, 4:3};
$('jogs').innerHTML=JOINTS.map(([n,i])=>`
  <div class="jog">
    <button class="ghost jogbtn" data-j="${i}" data-d="-${STEP[i]}">&minus;</button>
    <span>${n}</span>
    <button class="ghost jogbtn" data-j="${i}" data-d="${STEP[i]}">+</button>
  </div>`).join('');
let jogTimer=null;
function jogStart(j,d){ jogTimerStop(); jog(j,d); jogTimer=setInterval(()=>jog(j,d), 100); }
function jogTimerStop(){ if(jogTimer){clearInterval(jogTimer);jogTimer=null;} }
document.querySelectorAll('.jogbtn').forEach(b=>{
  const j=+b.dataset.j, d=+b.dataset.d;
  b.addEventListener('mousedown',   e=>{e.preventDefault();jogStart(j,d);});
  b.addEventListener('touchstart',  e=>{e.preventDefault();jogStart(j,d);});
  b.addEventListener('mouseup',     jogTimerStop);
  b.addEventListener('mouseleave',  jogTimerStop);
  b.addEventListener('touchend',    jogTimerStop);
  b.addEventListener('touchcancel', jogTimerStop);
});
connect();
</script>
</body></html>
)HTML";
