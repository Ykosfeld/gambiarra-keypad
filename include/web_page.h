#pragma once
#include <Arduino.h>

const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Gambiarra Keypad | Configuração</title>
  <style>
    :root {
      --bg: #0f1117;
      --card-bg: #1a1d27;
      --card-header: #212534;
      --btn-bg: #272c3d;
      --border: #2e3448;
      --primary: #9333ea;
      --primary-hover: #a855f7;
      --primary-light: rgba(147, 51, 234, 0.15);
      --accent: #06b6d4;
      --success: #10b981;
      --danger: #ef4444;
      --danger-hover: #dc2626;
      --text: #f3f4f6;
      --text-muted: #9ca3af;
      --radius: 10px;
    }

    * { box-sizing: border-box; margin: 0; padding: 0; font-family: system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background-color: var(--bg); color: var(--text); padding-bottom: 90px; }

    header {
      background: linear-gradient(180deg, #161924 0%, var(--bg) 100%);
      padding: 20px 16px;
      border-bottom: 1px solid var(--border);
      position: sticky;
      top: 0;
      z-index: 50;
      backdrop-filter: blur(8px);
    }
    .header-container {
      max-width: 900px;
      margin: 0 auto;
      display: flex;
      justify-content: space-between;
      align-items: center;
      flex-wrap: wrap;
      gap: 12px;
    }
    .logo-area { display: flex; align-items: center; gap: 10px; }
    .logo-icon {
      font-size: 26px;
      background: var(--primary-light);
      padding: 6px 10px;
      border-radius: 8px;
      border: 1px solid var(--primary);
    }
    .logo-title h1 { font-size: 1.25rem; font-weight: 700; color: #fff; display: flex; align-items: center; gap: 6px; }
    .logo-title p { font-size: 0.8rem; color: var(--accent); }
    .header-actions { display: flex; gap: 8px; align-items: center; }

    .nav-tabs {
      display: flex;
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 8px;
      padding: 3px;
    }
    .tab-btn {
      background: transparent;
      border: none;
      color: var(--text-muted);
      padding: 6px 14px;
      font-size: 0.85rem;
      font-weight: 600;
      cursor: pointer;
      border-radius: 6px;
      transition: all 0.2s;
    }
    .tab-btn.active {
      background: var(--primary);
      color: #fff;
    }

    .main-container {
      max-width: 900px;
      margin: 20px auto;
      padding: 0 16px;
    }

    .banner-info {
      background: rgba(6, 182, 212, 0.08);
      border: 1px solid rgba(6, 182, 212, 0.25);
      border-radius: var(--radius);
      padding: 12px 16px;
      margin-bottom: 20px;
      display: flex;
      align-items: center;
      gap: 12px;
      font-size: 0.85rem;
      color: #bae6fd;
    }

    .mode-card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      margin-bottom: 20px;
      box-shadow: 0 4px 14px rgba(0,0,0,0.3);
      overflow: hidden;
      transition: border-color 0.2s;
    }
    .mode-card:hover { border-color: #3f4762; }
    .mode-header {
      background: var(--card-header);
      padding: 12px 16px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      border-bottom: 1px solid var(--border);
      gap: 10px;
      flex-wrap: wrap;
    }
    .mode-title-group {
      display: flex;
      align-items: center;
      gap: 10px;
      flex: 1;
      min-width: 200px;
    }
    .mode-badge {
      background: var(--primary-light);
      color: var(--primary-hover);
      font-size: 0.75rem;
      font-weight: 700;
      padding: 4px 8px;
      border-radius: 6px;
      border: 1px solid rgba(147, 51, 234, 0.3);
      white-space: nowrap;
    }
    .mode-name-input {
      background: #12141c;
      border: 1px solid var(--border);
      color: #fff;
      font-size: 0.95rem;
      font-weight: 600;
      padding: 6px 10px;
      border-radius: 6px;
      width: 100%;
      max-width: 240px;
    }
    .mode-name-input:focus {
      outline: none;
      border-color: var(--primary);
    }
    .mode-controls {
      display: flex;
      gap: 6px;
    }
    .btn-icon {
      background: var(--btn-bg);
      border: 1px solid var(--border);
      color: var(--text-muted);
      width: 32px;
      height: 32px;
      border-radius: 6px;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 0.9rem;
      transition: all 0.15s;
    }
    .btn-icon:hover { color: #fff; border-color: #4a5270; }
    .btn-icon.delete:hover { background: var(--danger); border-color: var(--danger); color: #fff; }

    .keys-grid {
      display: grid;
      grid-template-columns: repeat(3, 1fr);
      gap: 14px;
      padding: 16px;
    }
    @media (max-width: 768px) {
      .keys-grid { grid-template-columns: 1fr; }
    }

    .key-slot {
      background: #141620;
      border: 1px solid #282d3f;
      border-radius: 8px;
      padding: 14px;
      display: flex;
      flex-direction: column;
      gap: 10px;
      position: relative;
    }
    .key-slot-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
    }
    .key-label {
      font-size: 0.8rem;
      font-weight: 700;
      color: var(--accent);
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }

    label { font-size: 0.75rem; color: var(--text-muted); font-weight: 600; margin-bottom: 2px; display: block; }
    select, input[type="text"] {
      width: 100%;
      background: #1b1e2a;
      border: 1px solid var(--border);
      color: #fff;
      padding: 8px 10px;
      border-radius: 6px;
      font-size: 0.85rem;
      transition: border-color 0.15s;
    }
    select:focus, input[type="text"]:focus {
      outline: none;
      border-color: var(--primary);
    }

    .mods-container {
      display: flex;
      flex-wrap: wrap;
      gap: 5px;
      margin-top: 4px;
    }
    .mod-pill {
      background: #1f2333;
      border: 1px solid var(--border);
      color: var(--text-muted);
      padding: 4px 8px;
      border-radius: 5px;
      font-size: 0.72rem;
      font-weight: 700;
      cursor: pointer;
      user-select: none;
      transition: all 0.15s;
    }
    .mod-pill.active {
      background: var(--primary);
      border-color: var(--primary-hover);
      color: #fff;
      box-shadow: 0 0 8px rgba(147, 51, 234, 0.4);
    }

    .preview-box {
      background: #10121a;
      border: 1px dashed #2d3348;
      border-radius: 6px;
      padding: 8px;
      font-size: 0.78rem;
      text-align: center;
      color: #cbd5e1;
      font-family: monospace;
      min-height: 32px;
      display: flex;
      align-items: center;
      justify-content: center;
    }

    .btn-add-mode {
      width: 100%;
      background: #171a26;
      border: 2px dashed var(--border);
      color: var(--text-muted);
      padding: 16px;
      border-radius: var(--radius);
      font-size: 0.95rem;
      font-weight: 600;
      cursor: pointer;
      transition: all 0.2s;
      display: flex;
      align-items: center;
      justify-content: center;
      gap: 8px;
    }
    .btn-add-mode:hover {
      border-color: var(--primary);
      color: #fff;
      background: var(--card-bg);
    }

    /* JSON EDITOR */
    #json-view { display: none; }
    .json-card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 16px;
    }
    textarea#json-input {
      width: 100%;
      height: 450px;
      background: #12141d;
      color: #4ade80;
      border: 1px solid var(--border);
      border-radius: 6px;
      padding: 12px;
      font-family: "Courier New", Courier, monospace;
      font-size: 0.85rem;
      line-height: 1.4;
      resize: vertical;
    }
    textarea#json-input:focus { outline: none; border-color: var(--primary); }
    .json-actions {
      display: flex;
      gap: 10px;
      margin-top: 10px;
    }

    /* FIXED BOTTOM BAR */
    .bottom-bar {
      position: fixed;
      bottom: 0;
      left: 0;
      right: 0;
      background: rgba(22, 25, 36, 0.92);
      backdrop-filter: blur(10px);
      border-top: 1px solid var(--border);
      padding: 12px 16px;
      z-index: 100;
    }
    .bottom-bar-inner {
      max-width: 900px;
      margin: 0 auto;
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 12px;
    }
    .btn-primary {
      background: var(--primary);
      color: #fff;
      border: none;
      padding: 10px 22px;
      font-size: 0.95rem;
      font-weight: 700;
      border-radius: 8px;
      cursor: pointer;
      display: flex;
      align-items: center;
      gap: 8px;
      transition: background-color 0.2s, transform 0.1s;
      box-shadow: 0 4px 12px rgba(147, 51, 234, 0.35);
    }
    .btn-primary:hover { background: var(--primary-hover); }
    .btn-primary:active { transform: scale(0.98); }
    .btn-secondary {
      background: var(--btn-bg);
      border: 1px solid var(--border);
      color: var(--text);
      padding: 8px 16px;
      font-size: 0.85rem;
      font-weight: 600;
      border-radius: 8px;
      cursor: pointer;
      transition: all 0.2s;
    }
    .btn-secondary:hover { background: #32394d; }

    /* TOAST */
    #toast {
      position: fixed;
      bottom: 80px;
      right: 20px;
      background: var(--success);
      color: #fff;
      padding: 12px 20px;
      border-radius: 8px;
      font-weight: 600;
      font-size: 0.9rem;
      box-shadow: 0 6px 16px rgba(0,0,0,0.4);
      transform: translateY(100px);
      opacity: 0;
      transition: all 0.3s cubic-bezier(0.175, 0.885, 0.32, 1.275);
      z-index: 200;
      display: flex;
      align-items: center;
      gap: 8px;
    }
    #toast.show {
      transform: translateY(0);
      opacity: 1;
    }
    #toast.error { background: var(--danger); }
  </style>
</head>
<body>

  <header>
    <div class="header-container">
      <div class="logo-area">
        <div class="logo-icon">&#x2328;</div>
        <div class="logo-title">
          <h1>Gambiarra Keypad</h1>
          <p>gambiarra.local &bull; Configura&ccedil;&atilde;o</p>
        </div>
      </div>
      <div class="header-actions">
        <div class="nav-tabs">
          <button class="tab-btn active" id="tab-visual" onclick="switchTab('visual')">Visual</button>
          <button class="tab-btn" id="tab-json" onclick="switchTab('json')">{ } JSON</button>
        </div>
      </div>
    </div>
  </header>

  <main class="main-container">
    <div class="banner-info">
      <div>&#x1F4A1;</div>
      <div>
        Acesse via <strong>http://gambiarra.local</strong> ou <strong>http://192.168.4.1</strong>.
        Ap&oacute;s salvar, o ESP32 atualiza a mem&oacute;ria e o display OLED instantaneamente!
      </div>
    </div>

    <!-- VISUAL VIEW -->
    <div id="visual-view">
      <div id="modes-list"></div>
      <button class="btn-add-mode" onclick="addMode()">
        <span>+</span> Adicionar Novo Modo
      </button>
    </div>

    <!-- JSON VIEW -->
    <div id="json-view">
      <div class="json-card">
        <label for="json-input" style="font-size: 0.9rem; margin-bottom: 8px;">Edi&ccedil;&atilde;o Direta do JSON:</label>
        <textarea id="json-input" spellcheck="false"></textarea>
        <div class="json-actions">
          <button class="btn-secondary" onclick="formatJson()">Formatar JSON</button>
          <button class="btn-secondary" onclick="copyJson()">Copiar para Área de Transferência</button>
        </div>
      </div>
    </div>
  </main>

  <div class="bottom-bar">
    <div class="bottom-bar-inner">
      <button class="btn-secondary" onclick="resetDefaults()">Restaurar Padr&otilde;es</button>
      <button class="btn-primary" onclick="salvarConfig()">
        <span>&#x1F4BE;</span> Salvar no Keypad
      </button>
    </div>
  </div>

  <div id="toast"></div>

  <script>
    let configData = { modes: [] };

    const KEY_OPTIONS = [
      { group: "Setas Direcionais", items: [
        { val: "DOWN", label: "Seta para Baixo (DOWN)" },
        { val: "UP", label: "Seta para Cima (UP)" },
        { val: "LEFT", label: "Seta para Esquerda (LEFT)" },
        { val: "RIGHT", label: "Seta para Direita (RIGHT)" }
      ]},
      { group: "Ações do Sistema", items: [
        { val: "ENTER", label: "Enter / Return" },
        { val: "SPACE", label: "Espaço (SPACE)" },
        { val: "ESC", label: "Esc (Escape)" },
        { val: "TAB", label: "Tab" },
        { val: "BACKSPACE", label: "Backspace" },
        { val: "DELETE", label: "Delete" }
      ]},
      { group: "Navegação", items: [
        { val: "PAGE_UP", label: "Page Up" },
        { val: "PAGE_DOWN", label: "Page Down" },
        { val: "HOME", label: "Home" },
        { val: "END", label: "End" }
      ]},
      { group: "Teclas de Função", items: [
        { val: "F1", label: "F1" }, { val: "F2", label: "F2" },
        { val: "F3", label: "F3" }, { val: "F4", label: "F4" },
        { val: "F5", label: "F5" }, { val: "F6", label: "F6" },
        { val: "F7", label: "F7" }, { val: "F8", label: "F8" },
        { val: "F9", label: "F9" }, { val: "F10", label: "F10" },
        { val: "F11", label: "F11" }, { val: "F12", label: "F12" }
      ]},
      { group: "Letras (A - Z)", items: "ABCDEFGHIJKLMNOPQRSTUVWXYZ".split("").map(c => ({ val: c, label: `Tecla ${c}` })) },
      { group: "Números (0 - 9)", items: "0123456789".split("").map(c => ({ val: c, label: `Número ${c}` })) }
    ];

    function showToast(msg, isError = false) {
      const t = document.getElementById('toast');
      t.innerText = msg;
      t.className = isError ? 'show error' : 'show';
      setTimeout(() => { t.className = ''; }, 3200);
    }

    function switchTab(tab) {
      const vView = document.getElementById('visual-view');
      const jView = document.getElementById('json-view');
      const tVis = document.getElementById('tab-visual');
      const tJson = document.getElementById('tab-json');

      if (tab === 'visual') {
        try {
          const raw = document.getElementById('json-input').value;
          if (raw.trim()) {
            configData = JSON.parse(raw);
            renderModes();
          }
        } catch (e) {
          showToast('Erro no JSON: sintaxe inválida', true);
          return;
        }
        vView.style.display = 'block';
        jView.style.display = 'none';
        tVis.classList.add('active');
        tJson.classList.remove('active');
      } else {
        syncJsonFromVisual();
        vView.style.display = 'none';
        jView.style.display = 'block';
        tVis.classList.remove('active');
        tJson.classList.add('active');
      }
    }

    function syncJsonFromVisual() {
      document.getElementById('json-input').value = JSON.stringify(configData, null, 2);
    }

    function loadConfig() {
      fetch('/api/config')
        .then(r => r.json())
        .then(data => {
          configData = data;
          if (!configData.modes) configData.modes = [];
          renderModes();
          syncJsonFromVisual();
        })
        .catch(err => {
          console.error(err);
          showToast('Falha ao carregar configuração do ESP32', true);
        });
    }

    function renderModes() {
      const container = document.getElementById('modes-list');
      container.innerHTML = '';

      if (configData.modes.length === 0) {
        container.innerHTML = '<p style="text-align:center; padding:30px; color:#888;">Nenhum modo configurado. Clique abaixo para adicionar!</p>';
        return;
      }

      configData.modes.forEach((mode, mIdx) => {
        const card = document.createElement('div');
        card.className = 'mode-card';
        card.innerHTML = `
          <div class="mode-header">
            <div class="mode-title-group">
              <span class="mode-badge">Modo ${mIdx + 1}</span>
              <input type="text" class="mode-name-input" value="${escapeHtml(mode.name || '')}" 
                     placeholder="Nome do Modo" onchange="updateModeName(${mIdx}, this.value)" />
            </div>
            <div class="mode-controls">
              <button class="btn-icon" title="Mover para Cima" onclick="moveMode(${mIdx}, -1)">&#x25B2;</button>
              <button class="btn-icon" title="Mover para Baixo" onclick="moveMode(${mIdx}, 1)">&#x25BC;</button>
              <button class="btn-icon delete" title="Excluir Modo" onclick="deleteMode(${mIdx})">&#x1F5D1;</button>
            </div>
          </div>
          <div class="keys-grid">
            ${renderKeySlot(mIdx, 'b1', 'Botão 1 (Esq)', mode.b1)}
            ${renderKeySlot(mIdx, 'b2', 'Botão 2 (Meio)', mode.b2)}
            ${renderKeySlot(mIdx, 'b3', 'Botão 3 (Dir)', mode.b3)}
          </div>
        `;
        container.appendChild(card);
      });
    }

    function renderKeySlot(mIdx, btnKey, btnTitle, act = {}) {
      const type = act.type || 'none';
      const val = act.val || '';
      const key = act.key || '';
      const mods = act.mods || [];

      return `
        <div class="key-slot" id="slot-${mIdx}-${btnKey}">
          <div class="key-slot-header">
            <span class="key-label">${btnTitle}</span>
          </div>

          <div>
            <label>Tipo de Ação</label>
            <select onchange="updateKeyType(${mIdx}, '${btnKey}', this.value)">
              <option value="none" ${type === 'none' ? 'selected' : ''}>⚪ Nenhuma Ação</option>
              <option value="media" ${type === 'media' ? 'selected' : ''}>🎵 Controle de Mídia</option>
              <option value="macro" ${type === 'macro' ? 'selected' : ''}>⌨️ Macro de Teclado</option>
            </select>
          </div>

          <!-- PAINEL MIDIA -->
          <div class="media-fields" style="display: ${type === 'media' ? 'block' : 'none'};">
            <label>Ação Multimídia</label>
            <select onchange="updateKeyMedia(${mIdx}, '${btnKey}', this.value)">
              <option value="PREV" ${val === 'PREV' ? 'selected' : ''}>⏮️ Faixa Anterior (PREV)</option>
              <option value="PLAY_PAUSE" ${val === 'PLAY_PAUSE' ? 'selected' : ''}>⏯️ Play / Pause (PLAY_PAUSE)</option>
              <option value="NEXT" ${val === 'NEXT' ? 'selected' : ''}>⏭️ Próxima Faixa (NEXT)</option>
              <option value="VOL_UP" ${val === 'VOL_UP' ? 'selected' : ''}>🔊 Aumentar Volume (VOL_UP)</option>
              <option value="VOL_DOWN" ${val === 'VOL_DOWN' ? 'selected' : ''}>🔉 Diminuir Volume (VOL_DOWN)</option>
              <option value="MUTE" ${val === 'MUTE' ? 'selected' : ''}>🔇 Mudo (MUTE)</option>
            </select>
          </div>

          <!-- PAINEL MACRO -->
          <div class="macro-fields" style="display: ${type === 'macro' ? 'block' : 'none'};">
            <label>Modificadores</label>
            <div class="mods-container">
              ${['CTRL', 'SHIFT', 'ALT', 'GUI'].map(mod => `
                <div class="mod-pill ${mods.includes(mod) ? 'active' : ''}" 
                     onclick="toggleMod(${mIdx}, '${btnKey}', '${mod}')">${mod === 'GUI' ? 'WIN / GUI' : mod}</div>
              `).join('')}
            </div>

            <label style="margin-top: 8px;">Tecla Principal</label>
            <select onchange="updateKeyKey(${mIdx}, '${btnKey}', this.value)">
              ${renderKeyOptions(key)}
            </select>
          </div>

          <div class="preview-box">
            ${renderPreview(act)}
          </div>
        </div>
      `;
    }

    function renderKeyOptions(selectedKey) {
      let html = '<option value="">(Nenhuma Tecla)</option>';
      KEY_OPTIONS.forEach(grp => {
        html += `<optgroup label="${grp.group}">`;
        grp.items.forEach(it => {
          const isSel = (it.val.toUpperCase() === (selectedKey || '').toUpperCase()) ? 'selected' : '';
          html += `<option value="${it.val}" ${isSel}>${it.label}</option>`;
        });
        html += `</optgroup>`;
      });
      return html;
    }

    function renderPreview(act = {}) {
      if (act.type === 'media') {
        const labels = {
          'PREV': '⏮️ Faixa Anterior',
          'PLAY_PAUSE': '⏯️ Play / Pause',
          'NEXT': '⏭️ Próxima Faixa',
          'VOL_UP': '🔊 Volume +',
          'VOL_DOWN': '🔉 Volume -',
          'MUTE': '🔇 Mudo'
        };
        return labels[act.val] || act.val || 'Mídia';
      }
      if (act.type === 'macro') {
        const parts = [...(act.mods || [])];
        if (act.key) parts.push(act.key);
        if (parts.length === 0) return '<span style="color:#666;">(Nenhuma tecla)</span>';
        return parts.map(p => `[${p}]`).join(' + ');
      }
      return '<span style="color:#555;">Desativado</span>';
    }

    function updateModeName(mIdx, val) {
      configData.modes[mIdx].name = val;
      syncJsonFromVisual();
    }

    function updateKeyType(mIdx, btnKey, type) {
      const act = configData.modes[mIdx][btnKey] || {};
      act.type = type;
      if (type === 'media' && !act.val) act.val = 'PLAY_PAUSE';
      if (type === 'macro' && !act.key) act.key = 'DOWN';
      configData.modes[mIdx][btnKey] = act;
      renderModes();
      syncJsonFromVisual();
    }

    function updateKeyMedia(mIdx, btnKey, val) {
      configData.modes[mIdx][btnKey].val = val;
      renderModes();
      syncJsonFromVisual();
    }

    function updateKeyKey(mIdx, btnKey, val) {
      configData.modes[mIdx][btnKey].key = val;
      renderModes();
      syncJsonFromVisual();
    }

    function toggleMod(mIdx, btnKey, mod) {
      const act = configData.modes[mIdx][btnKey];
      if (!act.mods) act.mods = [];
      const idx = act.mods.indexOf(mod);
      if (idx >= 0) act.mods.splice(idx, 1);
      else act.mods.push(mod);
      renderModes();
      syncJsonFromVisual();
    }

    function moveMode(mIdx, dir) {
      const target = mIdx + dir;
      if (target < 0 || target >= configData.modes.length) return;
      const temp = configData.modes[mIdx];
      configData.modes[mIdx] = configData.modes[target];
      configData.modes[target] = temp;
      renderModes();
      syncJsonFromVisual();
    }

    function deleteMode(mIdx) {
      if (configData.modes.length <= 1) {
        if (!confirm("Excluir o único modo existente?")) return;
      }
      configData.modes.splice(mIdx, 1);
      renderModes();
      syncJsonFromVisual();
    }

    function addMode() {
      configData.modes.push({
        name: "Novo Modo",
        b1: { type: "macro", key: "c", mods: ["CTRL"], val: "" },
        b2: { type: "macro", key: "v", mods: ["CTRL"], val: "" },
        b3: { type: "media", val: "PLAY_PAUSE", key: "", mods: [] }
      });
      renderModes();
      syncJsonFromVisual();
      window.scrollTo({ top: document.body.scrollHeight, behavior: 'smooth' });
    }

    function formatJson() {
      try {
        const val = JSON.parse(document.getElementById('json-input').value);
        document.getElementById('json-input').value = JSON.stringify(val, null, 2);
        showToast("JSON formatado!");
      } catch (e) {
        showToast("JSON inválido!", true);
      }
    }

    function copyJson() {
      const txt = document.getElementById('json-input');
      txt.select();
      navigator.clipboard.writeText(txt.value).then(() => {
        showToast("JSON copiado para a área de transferência!");
      }).catch(() => {
        showToast("Falha ao copiar", true);
      });
    }

    function resetDefaults() {
      if (!confirm("Deseja realmente restaurar os 3 modos padrão de fábrica?")) return;
      configData = {
        modes: [
          {
            name: "Midia",
            b1: { type: "media", val: "PREV", key: "", mods: [] },
            b2: { type: "media", val: "PLAY_PAUSE", key: "", mods: [] },
            b3: { type: "media", val: "NEXT", key: "", mods: [] }
          },
          {
            name: "Discord",
            b1: { type: "macro", key: "DOWN", mods: ["GUI"], val: "" },
            b2: { type: "macro", key: "UP", mods: ["GUI"], val: "" },
            b3: { type: "none", key: "", mods: [], val: "" }
          },
          {
            name: "Estudo",
            b1: { type: "macro", key: "RIGHT", mods: ["SHIFT", "GUI"], val: "" },
            b2: { type: "none", key: "", mods: [], val: "" },
            b3: { type: "none", key: "", mods: [], val: "" }
          }
        ]
      };
      renderModes();
      syncJsonFromVisual();
      showToast("Padrões restaurados na tela! Clique em Salvar para gravar.");
    }

    function salvarConfig() {
      let payload;
      // Garante sincronia antes de salvar
      if (document.getElementById('tab-json').classList.contains('active')) {
        try {
          payload = JSON.parse(document.getElementById('json-input').value);
          configData = payload;
        } catch (e) {
          showToast("JSON inválido! Corrija os erros antes de salvar.", true);
          return;
        }
      } else {
        payload = configData;
      }

      const bodyStr = JSON.stringify(payload);

      fetch('/api/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: bodyStr
      })
      .then(r => {
        if (!r.ok) throw new Error("Status " + r.status);
        return r.text();
      })
      .then(() => {
        showToast("✅ Salvo com sucesso no ESP32!");
      })
      .catch(err => {
        console.error(err);
        showToast("❌ Erro ao salvar configurações!", true);
      });
    }

    function escapeHtml(str) {
      return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
    }

    window.onload = loadConfig;
  </script>
</body>
</html>
)rawliteral";
