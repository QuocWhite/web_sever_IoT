/* ESP32 IoT Monitor - CSS for firmware
   Keep in sync with style.css */

#ifndef STYLE_H
#define STYLE_H

const char STYLE_CSS[] PROGMEM = R"css(
:root{--bg:#0f1419;--surface:#1a2332;--border:#2d3a4d;--text:#e6edf3;--text-muted:#8b949e;--accent:#58a6ff;--accent-green:#3fb950;--accent-orange:#d29922;--accent-red:#f85149}
*{box-sizing:border-box}
body{font-family:'Outfit',sans-serif;background:var(--bg);color:var(--text);margin:0;padding:24px;min-height:100vh}
.banner{background:rgba(210,153,34,.15);border:1px solid var(--accent-orange);border-radius:8px;padding:12px 16px;margin-bottom:20px;font-size:.9rem;color:var(--accent-orange)}
.container{max-width:720px;margin:0 auto}
h1{font-size:1.75rem;font-weight:700;margin-bottom:24px;display:flex;align-items:center;gap:10px}
h1::before{content:'';width:8px;height:28px;background:linear-gradient(180deg,var(--accent),var(--accent-green));border-radius:4px}
.status-dot{width:8px;height:8px;border-radius:50%;background:var(--accent-green);animation:pulse 2s infinite}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.5}}
.cards{display:grid;grid-template-columns:repeat(auto-fit,minmax(160px,1fr));gap:16px;margin-bottom:24px}
.card{background:var(--surface);border:1px solid var(--border);border-radius:12px;padding:20px;text-align:center}
.card-title{font-size:.8rem;color:var(--text-muted);text-transform:uppercase;letter-spacing:.05em;margin-bottom:8px}
.card-value{font-family:'JetBrains Mono',monospace;font-size:1.75rem;font-weight:600}
.card-unit{font-size:.9rem;color:var(--text-muted)}
.badge{display:inline-block;padding:6px 14px;border-radius:20px;font-size:.85rem;font-weight:600}
.badge-safe{background:rgba(63,185,80,.2);color:var(--accent-green)}
.badge-danger{background:rgba(248,81,73,.2);color:var(--accent-red)}
.chart-card{background:var(--surface);border:1px solid var(--border);border-radius:12px;padding:20px;margin-bottom:24px}
.chart-card h3{font-size:1rem;font-weight:600;margin:0 0 16px 0;color:var(--text-muted)}
.chart-wrap{height:200px;position:relative}
.controls{background:var(--surface);border:1px solid var(--border);border-radius:12px;padding:24px}
.controls h3{font-size:1rem;font-weight:600;margin:0 0 20px 0;color:var(--text-muted)}
.gpio-row{display:flex;align-items:center;justify-content:space-between;padding:12px 0;border-bottom:1px solid var(--border)}
.gpio-row:last-of-type{border-bottom:none}
.gpio-label{font-family:'JetBrains Mono',monospace;font-size:.9rem}
.btn{display:inline-block;padding:10px 24px;border-radius:8px;font-family:'Outfit',sans-serif;font-size:.9rem;font-weight:600;text-decoration:none;cursor:pointer;border:none;transition:transform .15s,opacity .15s}
.btn:hover{opacity:.9;transform:translateY(-1px)}
.btn-on{background:var(--accent-green);color:#0d1117}
.btn-off{background:var(--border);color:var(--text);margin-left:8px}
.device-row{display:flex;align-items:center;gap:12px}
.device-icon{display:inline-flex;align-items:center;justify-content:center;color:var(--text-muted);transition:color .3s,filter .3s}
.device-icon svg{display:block}
.device-icon-bulb.lit{color:#ffd54f;filter:drop-shadow(0 0 8px rgba(255,213,79,.6))}
.device-icon-fan.spinning svg{animation:fan-spin 1s linear infinite}
@keyframes fan-spin{from{transform:rotate(0deg)}to{transform:rotate(360deg)}}
.device-icon-fan.spinning{color:var(--accent)}
)css";

#endif
