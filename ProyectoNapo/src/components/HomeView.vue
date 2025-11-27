<template>
  <div class="home-container">
    <div class="hero-bg" aria-hidden="true"></div>

    <header class="navbar">
      <h1 class="logo">🌊 Alertas Climáticas</h1>
    </header>

    <section class="hero content-card">
      <div class="hero-content">
        <h2>Monitoreo Hídrico en Tiempo Real</h2>
        <p>Sistema automatizado basado en sensores para la prevención de desbordamientos.</p>
      </div>
    </section>

    <section class="map-section content-card">
      <h3 class="section-title">🗺️ Mapa de Alertas de Desborde</h3>
      <div id="map" class="map"></div>

      <button class="btn-whatsapp primary-action" @click="activarWhatsApp">
        🔔 **Activar Alertas por WhatsApp**
      </button>
    </section>

    <section class="news-section content-card">
      <h3 class="section-title">📰 Últimas Actualizaciones</h3>

      <div class="news-list">
        <div
          class="news-item alert-card"
          :class="`type-${n.type.toLowerCase()}`"
          v-for="n in news"
          :key="n.id"
        >
          <div class="content">
            <h4>{{ n.title }}</h4>
            <p>{{ n.text }}</p>
          </div>
          <span class="badge">{{ n.type.toUpperCase() }}</span>
        </div>
      </div>
    </section>

    <section class="twitter-section content-card">
      <h3 class="section-title">📢 Reportes y Noticias en Redes</h3>
      <p>Síguenos para información inmediata.</p>
    </section>

    <footer>
      <p>© 2025 Sistema de Alerta Río | Proyecto académico con fines educativos.</p>
    </footer>
  </div>
</template>

<script setup>
import { onMounted, ref } from 'vue'
import L from 'leaflet'
import 'leaflet/dist/leaflet.css'

import { db } from '@/firebase/firebase'
import { collection, onSnapshot, addDoc } from 'firebase/firestore'

// ---------------------------
// CONFIG GENERAL
// ---------------------------
const ALTURA_SENSOR = 10 // distancia desde el sensor hasta el agua en condiciones normales

const map = ref(null)
const markers = []

// Firebase collections
const alertCollection = collection(db, 'alertas')
const usersCollection = collection(db, 'usuariosWhatsapp')

// ---------------------------
// ICONO DE ALERTA
// ---------------------------
const news = ref([
  {
    id: 1,
    title: 'Reporte de Bajada de Nivel',
    text: 'El río ha regresado a niveles normales en la zona norte.',
    type: 'ok',
  },
  {
    id: 2,
    title: 'Pre-alerta por Lluvias Fuertes',
    text: 'Se esperan precipitaciones intensas en las próximas 6 horas.',
    type: 'alert',
  },
  {
    id: 3,
    title: 'Desbordamiento en Sector X',
    text: 'Se pide a los habitantes de la zona evacuar de inmediato.',
    type: 'danger',
  },
])
function makeIcon(color) {
  return L.divIcon({
    className: '',
    html: `
      <span style="
        background:${color};
        width:18px;
        height:18px;
        display:block;
        border-radius:50%;
        border:2px solid white;">
      </span>`,
    iconSize: [20, 20],
    iconAnchor: [10, 10],
  })
}

// ---------------------------
// REGISTRAR USUARIO WHATSAPP
// ---------------------------
async function activarWhatsApp() {
  const phone = prompt('Ingresa tu número en formato internacional (ej: 50371234567):')

  if (!phone || phone.length < 10) {
    alert('Número inválido')
    return
  }

  alert(
    'Paso 1: Guarda este número: +34 644 36 39 98\n\n' +
      "Paso 2: Envíale este mensaje: 'I allow callmebot to send me messages'\n\n" +
      'Después de eso presiona OK.',
  )

  await addDoc(usersCollection, {
    phone,
    registrado: true,
  })

  alert('¡Registrado correctamente!')
}

// ---------------------------
// AGREGAR MARCADOR EN MAPA
// ---------------------------
function addMarker(doc) {
  const data = doc.data()

  if (!data.lat || !data.lng || data.distancia === undefined) return

  const distancia = Number(data.distancia)

  if (isNaN(distancia)) {
    console.warn('Distancia inválida:', data.distancia)
    return
  }

  // Crecimiento real
  const crecimiento = Math.max(0, ALTURA_SENSOR - distancia)

  // Estado
  let estado = 'Tranquilo'
  if (crecimiento >= 8) estado = 'Peligro'
  else if (crecimiento >= 4) estado = 'Alerta'

  // Color
  const color = estado === 'Peligro' ? 'red' : estado === 'Alerta' ? 'orange' : 'green'

  // Marcar
  const marker = L.marker([data.lat, data.lng], {
    icon: makeIcon(color),
  })

  const tooltipText = `
    Estado: ${estado}
    Crecimiento: ${crecimiento.toFixed(1)} cm
  `.trim()

  marker.bindTooltip(tooltipText, {
    permanent: false,
    direction: 'top',
    offset: [0, -15],
  })

  marker.addTo(map.value)
  markers.push(marker)
}

// ---------------------------
// TIEMPO REAL
// ---------------------------
function enableRealtime() {
  onSnapshot(alertCollection, (snapshot) => {
    // limpiar
    markers.forEach((m) => map.value.removeLayer(m))
    markers.length = 0

    // cargar
    snapshot.forEach((doc) => addMarker(doc))
  })
}

// ---------------------------
// INICIALIZAR MAPA
// ---------------------------
onMounted(() => {
  map.value = L.map('map').setView([13.479453791020822, -88.17785764170928], 11)

  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '© OpenStreetMap contributors',
  }).addTo(map.value)

  // Desactivar agregar alertas con clic
  map.value.off('click')

  // Activar modo realtime
  enableRealtime()
})
</script>

<style scoped>
/* Fuente futurista / moderna */
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700;800&family=Outfit:wght@300;500;700;800&display=swap');

:root {
  --bg-dark: #0c0f14;
  --bg-card: #161a20;
  --bg-card-light: #1d232c;
  --txt-main: #f5faff;
  --txt-muted: #98a3b3;

  --green-main: #25d366;
  --green-ok: #4caf50;
  --orange-alert: #ff9800;
  --red-danger: #ff4d4d;

  --border: #262c35;

  --glow-green: 0 0 15px rgba(37, 211, 102, 0.45);
  --glow-red: 0 0 18px rgba(255, 77, 77, 0.55);
}

/* 🔥 Fondo animado con gradiente futurista */
.hero-bg {
  position: fixed;
  inset: 0;
  background:
    radial-gradient(circle at 20% 30%, #1a2330 0%, #0c0f14 55%),
    radial-gradient(circle at 80% 70%, #14212b 0%, #0c0f14 60%);
  background-size: 200% 200%;
  animation: bgMove 10s alternate-reverse infinite ease-in-out;
  z-index: -5;
}

@keyframes bgMove {
  0% {
    background-position: 0% 0%;
  }
  100% {
    background-position: 100% 100%;
  }
}

/* ===================================
   NAV BAR
   =================================== */
.navbar {
  padding: 1rem 2rem;
  background: rgba(22, 26, 32, 0.9);
  backdrop-filter: blur(14px);
  border-bottom: 1px solid var(--border);
  position: sticky;
  top: 0;
  z-index: 20;
}

.logo {
  font-size: 2rem;
  font-weight: 800;
  font-family: 'Outfit', sans-serif;
  color: var(--green-main);
  text-shadow: var(--glow-green);
}

/* ===================================
   HERO (PORTADA)
   =================================== */
.hero {
  text-align: center;
  padding: 4rem 1rem;
  width: min(1100px, 96%);
  margin: 2rem auto;

  border-radius: 20px;
  background: linear-gradient(135deg, #161a20cc, #1d232ccc);
  backdrop-filter: blur(25px);

  border: 1px solid var(--border);
  box-shadow: 0 10px 40px rgba(0, 0, 0, 0.45);
}

.hero h2 {
  font-size: clamp(2.1rem, 4vw, 3.3rem);
  color: var(--txt-main);
  font-weight: 800;
  margin-bottom: 0.5rem;
  font-family: 'Outfit', sans-serif;
}

.hero p {
  color: var(--txt-muted);
  font-size: 1.2rem;
  max-width: 700px;
  margin: 0 auto;
}

/* Estado general */
.status-indicator {
  margin-top: 1.5rem;
  padding: 1rem 1.6rem;
  border-radius: 12px;
  font-size: 1.2rem;

  border-left: 4px solid;
  background: #0f1318;
  box-shadow: 0 8px 25px rgba(0, 0, 0, 0.4);
}

.status-indicator.danger {
  border-left-color: var(--red-danger);
  color: var(--red-danger);
  text-shadow: var(--glow-red);
}

/* ===================================
   TARJETAS GLOBAL (Glass Neo)
   =================================== */
.glass-card {
  width: min(1100px, 96%);
  margin: 2rem auto;
  padding: 2rem;

  background: linear-gradient(135deg, #161a20e0, #1d232ce0);
  border-radius: 18px;
  border: 1px solid #2a303b;
  backdrop-filter: blur(18px);

  box-shadow: 0 14px 35px rgba(0, 0, 0, 0.45);
  transition:
    transform 0.3s ease,
    border 0.3s ease;
}

.glass-card:hover {
  transform: translateY(-3px);
  border-color: var(--green-main);
}

/* Títulos */
.section-title {
  font-size: 1.8rem;
  margin-bottom: 1.3rem;
  font-weight: 700;
  color: var(--txt-main);
  font-family: 'Outfit';
}

/* ===================================
   MAPA
   =================================== */
.map {
  height: clamp(300px, 50vh, 650px);
  width: 100%;
  border-radius: 14px;
  border: 1px solid var(--border);

  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.5);
}

.leaflet-container {
  background: #101419 !important;
}

/* ===================================
   BOTÓN WHATSAPP
   =================================== */
.btn-whatsapp {
  margin-top: 1.5rem;
  padding: 14px 24px;
  border-radius: 12px;

  background: var(--green-main);
  color: #0c0f14;
  font-weight: 800;
  font-size: 1.1rem;

  border: none;
  cursor: pointer;

  box-shadow: var(--glow-green);
  transition: 0.3s ease;
}

.btn-whatsapp:hover {
  background: #1ebe5e;
  transform: translateY(-2px);
}

/* ===================================
   LISTA DE ALERTAS (modern cards)
   =================================== */
.news-list {
  display: grid;
  gap: 1.4rem;
  grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
}

.alert-card {
  padding: 1.3rem;
  border-radius: 12px;
  background: #0f1318;

  border-left: 5px solid;
  transition: 0.25s ease;
  box-shadow: 0 8px 25px rgba(0, 0, 0, 0.35);
}

.alert-card:hover {
  transform: translateY(-4px);
  box-shadow: 0 10px 35px rgba(0, 0, 0, 0.45);
}

.alert-card h4 {
  color: var(--txt-main);
  font-weight: 600;
}

.alert-card p {
  color: var(--txt-muted);
  font-size: 0.9rem;
}

.alert-card .badge {
  padding: 0.35rem 0.8rem;
  font-size: 0.75rem;
  border-radius: 5px;
  font-weight: 700;
  text-transform: uppercase;
  color: #000;
}

.type-ok {
  border-left-color: var(--green-ok);
}
.type-alert {
  border-left-color: var(--orange-alert);
}
.type-danger {
  border-left-color: var(--red-danger);
}

.type-ok .badge {
  background: var(--green-ok);
}
.type-alert .badge {
  background: var(--orange-alert);
}
.type-danger .badge {
  background: var(--red-danger);
}

/* ===================================
   TWITTER SECTION
   =================================== */
.twitter-section p {
  color: var(--txt-muted);
}

/* ===================================
   FOOTER
   =================================== */
footer {
  margin-top: 2rem;
  padding: 1.2rem;
  text-align: center;
  color: var(--txt-muted);
  border-top: 1px solid var(--border);
  background: rgba(22, 26, 32, 0.7);
}
</style>
