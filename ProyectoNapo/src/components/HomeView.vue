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

      <!-- 🔥 Nuevo botón -->
      <button class="btn-whatsapp" @click="activarWhatsApp">
        🔔 Activar alertas por WhatsApp
      </button>

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
import { collection, onSnapshot, getDocs } from 'firebase/firestore'

const map = ref(null)
const markers = []

// Colecciones Firebase
const alertCollection = collection(db, 'alertas')
const usersCollection = collection(db, 'usuariosWhatsApp')

// ------------------------------
// 🔥 FUNCIÓN PARA ENVIAR WHATSAPP
// ------------------------------
async function sendWhatsApp(phone, message) {
  const apiKey = "3034091"  // ⬅️ PON AQUÍ TU API KEY DE CALLMEBOT

  const url = `https://api.callmebot.com/whatsapp.php?phone=${phone}&text=${encodeURIComponent(
    message
  )}&apikey=${apiKey}`

  try {
    await fetch(url)
    console.log("Mensaje enviado a:", phone)
  } catch (error) {
    console.error("Error enviando mensaje:", error)
  }
}

// ------------------------------
// 🔥 ENVÍO MASIVO A LA COMUNIDAD
// ------------------------------
async function enviarAlertaComunitaria(estado, crecimiento, sensorName) {
  const mensaje = `⚠️ ALERTA AUTOMÁTICA DEL SISTEMA ⚠️
Sensor: ${sensorName}
Estado detectado: ${estado}
Crecimiento del río: ${crecimiento} cm.
Por favor tomar precauciones.`

  const snapshot = await getDocs(usersCollection)

  snapshot.forEach((doc) => {
    const data = doc.data()
    sendWhatsApp(data.phone, mensaje)
  })
}


// ------------------------------
// CREAR ICONO
// ------------------------------
function makeIcon(color) {
  return L.divIcon({
    className: '',
    html: `<span style="
      background:${color};
      width:16px;
      height:16px;
      display:block;
      border-radius:50%;
      border:2px solid white;
    "></span>`,
    iconSize: [20, 20],
    iconAnchor: [10, 10],
  })
}

import { addDoc } from "firebase/firestore";

async function activarWhatsApp() {
  const phone = prompt("Ingresa tu número de WhatsApp (ej: 50371234567):");

  if (!phone || phone.length < 10) {
    alert("Número inválido");
    return;
  }

  alert(
    "Paso 1: Guarda este número en tus contactos: +34 644 36 39 98\n\n" +
      "Paso 2: Envíale un mensaje: 'I allow callmebot to send me messages'\n\n" +
      "Paso 3: Cuando lo hagas, presiona OK para guardar tu número en el sistema."
  );

  await addDoc(usersCollection, {
    phone: phone,
    name: "Usuario registrado"
  });

  alert("¡Listo! Recibirás alertas cuando el sistema detecte riesgo.");
}


// ------------------------------
// AGREGAR MARCADOR Y ENVIAR ALERTA SI ES NECESARIO
// ------------------------------
function addMarker(doc) {
  const data = doc.data()
  if (!data.lat || !data.lng || data.distancia === undefined) return

  const ALTURA_SENSOR = 10 // cm
  const distancia = Number(data.distancia)
  const crecimiento = Math.max(0, ALTURA_SENSOR - distancia)

  // Determinar estado
  let estado = 'Tranquilo'
  if (crecimiento > 8) estado = 'Peligro'
  else if (crecimiento > 4) estado = 'Alerta'

  const color =
    estado === 'Peligro' ? 'red' : estado === 'Alerta' ? 'orange' : 'green'

  const marker = L.marker([data.lat, data.lng], {
    icon: makeIcon(color),
  })

  marker.bindTooltip(
    `Estado: ${estado}<br>Crecimiento: ${crecimiento.toFixed(1)} cm`,
    {
      permanent: false,
      direction: 'top',
      offset: [0, -15],
    }
  )

  marker.addTo(map.value)
  markers.push(marker)

  // ---------------------------------------------------
  // 🔥 SI ES ALERTA O PELIGRO → ENVIAR WHATSAPP
  // ---------------------------------------------------
  if (estado === 'Alerta' || estado === 'Peligro') {
   const crecimientoFormateado = crecimiento.toFixed(1)
  enviarAlertaComunitaria(estado, crecimientoFormateado, doc.id)
  }
}

// ------------------------------
// ACTIVAR LECTURA EN TIEMPO REAL
// ------------------------------
function enableRealtime() {
  onSnapshot(alertCollection, (snapshot) => {
    markers.forEach((m) => map.value.removeLayer(m))
    markers.length = 0

    snapshot.forEach((doc) => addMarker(doc))
  })
}

// ------------------------------
// UBICAR USUARIO
// ------------------------------
function locateUser() {
  map.value.locate({ setView: true, maxZoom: 14 })
}

// ------------------------------
// INICIALIZAR MAPA
// ------------------------------
onMounted(() => {
  map.value = L.map('map').setView(
    [13.479453791020822, -88.17785764170928],
    11
  )

  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '© OpenStreetMap contributors',
  }).addTo(map.value)

  map.value.off('click')

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

.btn-whatsapp {
  background: #25d366;
  color: white;
  padding: 12px 18px;
  border-radius: 10px;
  font-size: 1rem;
  border: none;
  cursor: pointer;
  margin-top: 15px;
}

.btn-whatsapp:hover {
  background: #1ebe5d;
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
