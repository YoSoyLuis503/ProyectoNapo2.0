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

      <div class="map-controls">
        <button class="btn" @click="locateUser">Mi ubicación</button>
        <button class="btn secondary" @click="clearAlerts">Borrar alertas</button>
      </div>

      <p class="hint">Haz clic en el mapa para agregar una alerta.</p>
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
import { onMounted, ref } from "vue";
import L from "leaflet";
import "leaflet/dist/leaflet.css";

import { db } from "@/firebase/firebase";
import { collection, onSnapshot } from "firebase/firestore";

const map = ref(null);
const markers = [];
const alertCollection = collection(db, "alertas");

// Crear ícono sin CSS scoped
function makeIcon(color) {
  return L.divIcon({
    className: "",
    html: `<span style="
      background:${color};
      width:16px;
      height:16px;
      display:block;
      border-radius:50%;
      border:2px solid white;
    "></span>`,
    iconSize: [20, 20],
    iconAnchor: [10, 10]
  });
}

// Renderizar un marcador desde Firebase
function addMarker(doc) {
  const data = doc.data();

  if (!data.lat || !data.lng || !data.state) return;

  const color =
    data.state === "danger"
      ? "red"
      : data.state === "alert"
      ? "orange"
      : "green";

  const marker = L.marker([data.lat, data.lng], {
    icon: makeIcon(color)
  });

  // Tooltip con descripción o estado
  const tooltipText = data.text
    ? `${data.state.toUpperCase()} — ${data.text}`
    : data.state.toUpperCase();

  marker.bindTooltip(tooltipText, {
    permanent: false,
    direction: "top",
    offset: [0, -15]
  });

  marker.addTo(map.value);
  markers.push(marker);
}

// Listener en tiempo real
function enableRealtime() {
  onSnapshot(alertCollection, (snapshot) => {
    // Eliminar marcadores viejos
    markers.forEach((m) => map.value.removeLayer(m));
    markers.length = 0;

    // Agregar nuevos
    snapshot.forEach((doc) => addMarker(doc));
  });
}

function locateUser() {
  map.value.locate({ setView: true, maxZoom: 14 });
}

onMounted(() => {
  // Configurar mapa
  map.value = L.map("map").setView([13.479453791020822, -88.17785764170928], 11);

  L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
    attribution: "© OpenStreetMap contributors"
  }).addTo(map.value);

  // ❗ Desactivar creación de alertas (no click)
  map.value.off("click");

  // Activar lectura de Firebase en tiempo real
  enableRealtime();
});
</script>




<style scoped>

.leaflet-container {
  width: 100% !important;
  height: 350px !important;
}

/* Decorative hero background (replaces video) */
.hero-bg {
  position: fixed;
  inset: 0;
  background: linear-gradient(180deg, rgba(1,40,60,0.75) 0%, rgba(2,44,67,0.85) 60%);
  background-blend-mode: multiply;
  z-index: -2;
}

/* Container and general layout */
.home-container {
  color: var(--color-text, #e6f7f5);
  font-family: "Source Sans Pro", sans-serif;
  backdrop-filter: blur(4px);
  width: 100%;
  max-width: 100%;
  margin: 0;
  box-sizing: border-box;
}

/* Navbar */
.navbar {
  padding: clamp(.5rem, 1.5vw, 1.2rem) var(--container-inline, 1rem);
  background: linear-gradient(90deg, rgba(6,73,90,0.85), rgba(4,47,80,0.85));
  position: sticky;
  top: 0;
  z-index: 10;
  box-shadow: 0 2px 10px rgba(2,20,30,0.35);
}
.logo {
  font-size: clamp(1.1rem, 2.6vw, 1.6rem);
  font-weight: 700;
  color: var(--accent, #2dd4bf);
}

/* Hero section */
.hero {
  text-align: center;
  padding-top: clamp(2rem, 6vw, 6rem);
  padding-bottom: clamp(1rem, 3vw, 3rem);
  background: linear-gradient(180deg, rgba(255,255,255,0.02), rgba(0,0,0,0.06));
  margin-top: 1.5rem;
  border-radius: 12px;
  width: min(1200px, 96%);
  margin-left: auto;
  margin-right: auto;
  box-shadow: 0 6px 24px rgba(2,20,30,0.35);
  padding-inline: clamp(.75rem, 2.5vw, 2rem);
}
.hero h2 {
  font-size: clamp(1.25rem, 3.5vw, 2.5rem);
  margin-bottom: .5rem;
}
.hero p {
  font-size: clamp(.95rem, 1.6vw, 1.2rem);
  opacity: 0.9;
}

/* Map section */
.map-section {
  padding: clamp(.75rem, 2.5vw, 2.5rem);
  background: rgba(0, 0, 0, 0.55);
  margin-top: 1rem;
  display: flex;
  flex-direction: column;
  gap: 1rem;
  align-items: center;
}
.map {
  width: 100%;
  max-width: 100%;
  height: clamp(240px, 45vh, 600px);
  border-radius: 10px;
}

/* Buttons */
.map-controls { display:flex; gap:.5rem; flex-wrap:wrap; justify-content:center; }
.btn {
  background: var(--accent, #2dd4bf);
  color: #042031;
  padding: .6rem .9rem;
  border-radius: 8px;
  font-weight: 700;
  cursor: pointer;
  border: none;
  box-shadow: 0 4px 10px rgba(5,30,40,0.25);
}
.btn.secondary {
  background: var(--muted, #3b5566);
  color: #f7fbfb;
}
.hint {
  font-size: .95rem;
  opacity: 0.8;
  text-align: center;
}

/* News section */
.news-section {
  padding: clamp(.75rem, 2.5vw, 2.5rem);
  background: rgba(0, 0, 0, 0.5);
  margin-top: 1rem;
}
.news-list {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
  gap: 1rem;
}
.news-item {
  padding: .9rem;
  border-radius: 10px;
  background: linear-gradient(180deg, rgba(255,255,255,0.02), rgba(255,255,255,0.01));
  border: 1px solid rgba(255,255,255,0.03);
}
.news-item .badge {
  padding: .25rem .6rem;
  border-radius: 6px;
  font-size: .85rem;
  color: white;
}
.badge.ok { background: green; }
.badge.alert { background: orange; }
.badge.danger { background: red; }

/* Twitter */
.twitter-section {
  padding: clamp(.75rem, 2.5vw, 2.5rem);
  background: rgba(0, 0, 0, 0.55);
  margin-top: 1rem;
}

/* Footer */
footer {
  padding: 1rem;
  text-align: center;
  background: rgba(0, 0, 0, 0.35);
  margin-top: 1rem;
}

/* Media rules */
@media (min-width: 900px) {
  .map-section { align-items: stretch; }
  .map-controls { justify-content: flex-start; }
}

</style>
