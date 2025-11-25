<template>
  <div class="home-container">
    <div class="hero-bg" aria-hidden="true"></div>

    <!-- NAVBAR -->
    <header class="navbar">
      <h1 class="logo">Alertas Climáticas</h1>
    </header>

    <!-- HERO SECTION -->
    <section class="hero">
      <h2>Monitoreo de desbordamiento en tiempo real</h2>
      <p>Sistema automatizado basado en sensores y transmisión en redes.</p>
    </section>

    <!-- MAP + ALERTS SECTION -->
    <section class="map-section">
      <h3>Mapa de Alertas</h3>
      <div id="map" class="map"></div>

      <!-- <div class="map-controls">
        <button class="btn" @click="locateUser">Mi ubicación</button>
        <button class="btn secondary" @click="clearAlerts">Borrar alertas</button>
      </div>

      <p class="hint">Haz clic en el mapa para agregar una alerta.</p>-->
    </section>

    <!-- NEWS / TIMELINE SECTION -->
    <section class="news-section">
      <h3>Últimas Noticias</h3>

      <div class="news-list">
        <div class="news-item" v-for="n in news" :key="n.id">
          <h4>{{ n.title }}</h4>
          <p>{{ n.text }}</p>
          <span class="badge" :class="n.type">{{ n.type }}</span>
        </div>
      </div>
    </section>

    <!-- TWITTER SECTION -->
    <section class="twitter-section">
      <h3>Alertas en Redes</h3>
    </section>

    <!-- FOOTER -->
    <footer>
      <p>© 2025 Sistema de Alerta Río. Proyecto académico.</p>
    </footer>
  </div>
</template>

<script setup>
import { onMounted, ref } from 'vue'
import L from 'leaflet'
import 'leaflet/dist/leaflet.css'

import { db } from '@/firebase/firebase'
import { collection, onSnapshot } from 'firebase/firestore'

// --- Configuración ---
const ALTURA_SENSOR = 10 // ← cámbialo según tu instalación

// Mapa y marcadores
const map = ref(null)
const markers = []

// Colección Firestore
const alertCollection = collection(db, 'alertas')

// Crear ícono circular
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

function addMarker(doc) {
  const data = doc.data()

  // Validar campos esenciales
  if (!data.lat || !data.lng || data.distancia === undefined) return

  const distancia = Number(data.distancia)

  if (isNaN(distancia)) {
    console.warn('⚠ Valor inválido de distancia:', data.distancia)
    return
  }

  // 1) Crecimiento real del río
  const crecimiento = Math.max(0, ALTURA_SENSOR - distancia)

  // 2) Estado según crecimiento
  let estado = 'Normal'
  if (crecimiento >= 10) estado = 'Peligro'
  else if (crecimiento >= 5) estado = 'Alerta'

  // 3) Color del marcador
  const color = estado === 'Peligro' ? 'red' : estado === 'Alerta' ? 'orange' : 'green'

  // 4) Crear marcador
  const marker = L.marker([data.lat, data.lng], { icon: makeIcon(color) })

  // 5) Tooltip
  const tooltip = `
    <strong>${estado}</strong><br>
    Crecimiento: ${crecimiento.toFixed(1)} cm<br>
    Distancia cruda: ${distancia} cm
  `

  marker.bindTooltip(tooltip, {
    permanent: false,
    direction: 'top',
    offset: [0, -15],
  })

  marker.addTo(map.value)
  markers.push(marker)
}

// --- Escuchar Firestore en tiempo real ---
function enableRealtime() {
  onSnapshot(alertCollection, (snapshot) => {
    // limpiar marcadores anteriores
    markers.forEach((m) => map.value.removeLayer(m))
    markers.length = 0

    // agregar los nuevos
    snapshot.docs.forEach((doc) => addMarker(doc))
  })
}

onMounted(() => {
  // inicializar mapa
  map.value = L.map('map').setView([13.479453791020822, -88.17785764170928], 11)

  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '© OpenStreetMap contributors',
  }).addTo(map.value)

  // desactivar creación de alertas
  map.value.off('click')

  // activar Firebase realtime
  enableRealtime()
})
</script>

<style scoped>
/* Leaflet map container */
.leaflet-container {
  width: 100% !important;
  height: 350px !important;
  border-radius: 12px;
  overflow: hidden;
}

/* --- BACKGROUND GENERAL (suave, claro) --- */
.hero-bg {
  position: fixed;
  inset: 0;
  background: linear-gradient(180deg, rgba(240, 250, 245, 0.9) 0%, rgba(225, 245, 235, 0.95) 60%);
  z-index: -2;
}

/* --- CONTENEDOR PRINCIPAL --- */
.home-container {
  color: #0b3d2e;
  font-family: 'Source Sans Pro', sans-serif;
  width: 100%;
  max-width: 100%;
  margin: 0;
  box-sizing: border-box;
}

/* --- NAVBAR (clarita, ambiental) --- */
.navbar {
  padding: clamp(0.8rem, 1.8vw, 1.4rem) var(--container-inline, 1rem);
  background: #f4fbf7;
  border-bottom: 2px solid #cfe9d9;
  position: sticky;
  top: 0;
  z-index: 10;
}

/* Título principal */
.logo {
  font-size: clamp(1.6rem, 3.4vw, 2.6rem);
  font-weight: 800;
  color: #0b3d2e;
  letter-spacing: 1px;
}

/* --- HERO SECTION --- */
.hero {
  text-align: center;
  padding: 3rem 1rem;
  margin-top: 1.5rem;
  background: #ffffffaa;
  border-radius: 14px;
  width: min(1200px, 96%);
  margin-inline: auto;
  box-shadow: 0 4px 18px rgba(0, 0, 0, 0.08);
}

.hero h2 {
  font-size: clamp(1.8rem, 3.6vw, 2.8rem);
  font-weight: 700;
  color: #0f4b36;
}

.hero p {
  font-size: clamp(1.05rem, 1.8vw, 1.25rem);
  opacity: 0.8;
}

/* --- MAP SECTION --- */
.map-section {
  padding: 2rem;
  background: #ffffffaa;
  margin-top: 1.5rem;
  border-radius: 14px;
  box-shadow: 0 4px 18px rgba(0, 0, 0, 0.08);
  width: min(1200px, 96%);
  margin-inline: auto;
}

.map {
  height: clamp(240px, 45vh, 600px);
  width: 100%;
  border-radius: 12px;
}

/* Buttons */
.map-controls {
  display: flex;
  gap: 0.7rem;
  flex-wrap: wrap;
  justify-content: center;
  margin-top: 0.5rem;
}

.btn {
  background: #2e8b57;
  color: white;
  padding: 0.65rem 1rem;
  border-radius: 8px;
  font-weight: 700;
  cursor: pointer;
  border: none;
  transition: 0.2s;
}

.btn:hover {
  background: #256f45;
}

.btn.secondary {
  background: #c19a6b;
  color: white;
}

.btn.secondary:hover {
  background: #9d7a52;
}

.hint {
  font-size: 0.95rem;
  opacity: 0.75;
  text-align: center;
  margin-top: 0.5rem;
}

/* --- NEWS SECTION --- */
.news-section {
  padding: 2rem;
  background: #ffffffaa;
  margin-top: 1.5rem;
  border-radius: 14px;
  width: min(1200px, 96%);
  margin-inline: auto;
  box-shadow: 0 4px 18px rgba(0, 0, 0, 0.08);
}

.news-list {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
  gap: 1rem;
}

.news-item {
  padding: 1rem;
  border-radius: 12px;
  background: #f6fff8;
  border: 1px solid #cde8d6;
}

.news-item .badge {
  padding: 0.3rem 0.7rem;
  border-radius: 6px;
  font-size: 0.85rem;
  color: white;
}

.badge.ok {
  background: #2e8b57;
}
.badge.alert {
  background: #e4a11b;
}
.badge.danger {
  background: #c62828;
}

/* Twitter */
.twitter-section {
  padding: 2rem;
  background: #ffffffaa;
  margin-top: 1.5rem;
  border-radius: 14px;
  width: min(1200px, 96%);
  margin-inline: auto;
  box-shadow: 0 4px 18px rgba(0, 0, 0, 0.08);
}

/* Footer */
footer {
  padding: 1.2rem;
  text-align: center;
  background: #ffffffaa;
  margin-top: 1.5rem;
  border-top: 2px solid #cfe9d9;
}

@media (min-width: 900px) {
  .map-section {
    align-items: stretch;
  }
  .map-controls {
    justify-content: flex-start;
  }
}
</style>
