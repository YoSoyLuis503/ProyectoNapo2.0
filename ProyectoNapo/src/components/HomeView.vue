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
        🔔 Activar Alertas por WhatsApp
      </button>
    </section>

    <section class="content-card">
      <h3 class="section-title">📊 Últimas 48 mediciones</h3>

      <p style="font-size: 1.1rem; margin-bottom: 1rem">
        🌡️ <b>Promedio:</b> {{ promedioCrecimiento.toFixed(2) }} cm
      </p>

      <table class="table-metricas">
        <thead>
          <tr>
            <th>Crecimiento</th>
            <th>Estado</th>
            <th>Lat</th>
            <th>Lng</th>
            <th>Hora</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="m in metricas" :key="m.id">
            <td>{{ m.crecimiento.toFixed(1) }} cm</td>
            <td>{{ m.estado }}</td>
            <td>{{ m.lat }}</td>
            <td>{{ m.lng }}</td>
            <td>{{ new Date(m.timestamp).toLocaleTimeString() }}</td>
          </tr>
        </tbody>
      </table>
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
import { computed } from 'vue'
import 'leaflet/dist/leaflet.css'
import { db } from '@/firebase/firebase'
import {
  collection,
  onSnapshot,
  addDoc,
  getDocs,
  doc,
  query,
  orderBy,
  deleteDoc,
  limit,
} from 'firebase/firestore'

const metricas = ref([]) // ← Aquí guardaremos las últimas 48

const promedioCrecimiento = computed(() => {
  if (metricas.value.length === 0) return 0

  const suma = metricas.value.reduce((acc, m) => acc + m.crecimiento, 0)
  return suma / metricas.value.length
})

// --- CONFIGURACIÓN ---
const ALTURA_SENSOR = 10
const map = ref(null)
const markers = []
let alertaEnviada = false // Anti-spam

// --- FIREBASE ---
const alertCollection = collection(db, 'alertas')
const usersCollection = collection(db, 'usuariosWhatsApp')

// --- DATOS ESTÁTICOS ---
const news = ref([
  {
    id: 1,
    title: 'Reporte de Bajada de Nivel',
    text: 'El río ha regresado a niveles normales.',
    type: 'ok',
  },
  {
    id: 2,
    title: 'Pre-alerta por Lluvias',
    text: 'Se esperan precipitaciones intensas.',
    type: 'alert',
  },
  { id: 3, title: 'Desbordamiento', text: 'Evacuar zona baja inmediatamente.', type: 'danger' },
])

// --- UTILIDADES ---
function makeIcon(color) {
  return L.divIcon({
    className: '',
    html: `<span style="background:${color}; width:18px; height:18px; display:block; border-radius:50%; border:2px solid white; box-shadow: 0 0 5px rgba(0,0,0,0.5);"></span>`,
    iconSize: [20, 20],
    iconAnchor: [10, 10],
  })
}

// --- 1. REGISTRO DE USUARIO ---
async function activarWhatsApp() {
  const phone = prompt('Ingresa tu número (ej: 50371234567):')
  if (!phone || phone.length < 10) return alert('Número inválido')

  alert(
    'PASO 1: Guarda el número +34 694 29 84 96\nPASO 2: Envía por WhatsApp: "I allow callmebot to send me messages"\nPASO 3: Copia la API KEY que te responda el bot.',
  )

  const apikey = prompt('Pega aquí tu API KEY:')
  if (!apikey || apikey.length < 4) return alert('API KEY inválida.')

  await addDoc(usersCollection, {
    phone,
    apikey,
    registrado: true,
    name: 'Usuario Autoregistrado',
  })

  alert('¡Listo! Alertas activadas.')
}

async function cargarMetricas() {
  const q = query(metricasCollection, orderBy('timestamp', 'desc'), limit(48))
  const snapshot = await getDocs(q)

  metricas.value = snapshot.docs.map((doc) => ({
    id: doc.id,
    ...doc.data(),
  }))
}

// --- 2. ENVÍO DE ALERTAS MASIVAS ---
async function dispararAlertaMasiva(estado, nivel) {
  if (alertaEnviada) return

  console.log('⚠️ Disparando alertas...')
  const snapshot = await getDocs(usersCollection)
  if (snapshot.empty) return

  // 👇 AQUÍ ESTÁ EL CAMBIO EN EL MENSAJE 👇
  const mensaje = `🚨 *ALERTA RIO* 🚨\nEstado: *${estado}*\nNivel: ${nivel}cm\n\n⛔ *ATENCIÓN: CALLES CERRADAS EN LA ZONA* ⛔\n¡Tome precauciones!`

  const mensajeCodificado = encodeURIComponent(mensaje)
  let enviados = 0

  snapshot.forEach((doc) => {
    const u = doc.data()
    if (u.phone && u.apikey) {
      const url = `https://api.callmebot.com/whatsapp.php?phone=${u.phone}&text=${mensajeCodificado}&apikey=${u.apikey}`
      fetch(url).catch((err) => console.error(err))
      enviados++
    }
  })

  alertaEnviada = true
  alert(`⚠️ ALERTA: Mensaje de cierre de calles enviado a ${enviados} usuarios.`)
}

// --- 3. LOGICA DE SENSORES ---
function addMarker(doc) {
  const data = doc.data()
  const distRaw = data.Distancia !== undefined ? data.Distancia : data.distancia

  if (!data.lat || !data.lng || distRaw === undefined) return

  const distancia = Number(distRaw)
  if (isNaN(distancia)) return

  const crecimiento = Math.max(0, ALTURA_SENSOR - distancia)
  let estado = 'Tranquilo'

  if (crecimiento >= 8) estado = 'Peligro'
  else if (crecimiento >= 4) estado = 'Alerta'

  const color = estado === 'Peligro' ? 'red' : estado === 'Alerta' ? 'orange' : 'green'

  const marker = L.marker([data.lat, data.lng], { icon: makeIcon(color) })
  marker.bindTooltip(`Estado: ${estado} - Nivel: ${crecimiento.toFixed(1)}cm`, {
    permanent: false,
    direction: 'top',
    offset: [0, -15],
  })

  marker.addTo(map.value)
  markers.push(marker)

  if (estado === 'Peligro' || estado === 'Alerta') {
    dispararAlertaMasiva(estado, crecimiento.toFixed(1))
  }

  ultimaLectura = {
    distancia,
    estadoLocal: estado,
    lat: data.lat,
    lng: data.lng,
  }
}

// --- 4. MAPA Y REALTIME ---
function enableRealtime() {
  onSnapshot(alertCollection, (snapshot) => {
    markers.forEach((m) => map.value.removeLayer(m))
    markers.length = 0
    snapshot.forEach((doc) => addMarker(doc))
  })
}

let ultimaLectura = null

function capturarLecturaPeriodica() {
  setInterval(async () => {
    if (!ultimaLectura) return

    await guardarMetrica(ultimaLectura)
    await limitarMetricas()
  }, 5000) // <-- cada 5 segundos

  setInterval(() => {
    cargarMetricas()
  }, 5000)
}

// ---  MÉTRICAS ---
const metricasCollection = collection(db, 'metricas/sensor1/lecturas')

async function guardarMetrica(data) {
  try {
    await addDoc(metricasCollection, {
      crecimiento: ALTURA_SENSOR - data.distancia,
      estado: data.estadoLocal,
      lat: data.lat,
      lng: data.lng,
      timestamp: Date.now(),
    })

    console.log('📌 Métrica guardada')
  } catch (e) {
    console.error('❌ Error guardando métrica:', e)
  }
}

async function limitarMetricas() {
  const q = query(metricasCollection, orderBy('timestamp', 'asc'))
  const snapshot = await getDocs(q)

  if (snapshot.size <= 48) return

  const excedentes = snapshot.size - 48
  let borrados = 0

  snapshot.forEach(async (docu) => {
    if (borrados < excedentes) {
      await deleteDoc(doc(db, `metricas/sensor1/lecturas/${docu.id}`))
      borrados++
    }
  })
}

onMounted(() => {
  const centroMapa = [13.4742223, -88.1591111]
  map.value = L.map('map').setView(centroMapa, 13)
  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '© OpenStreetMap contributors',
  }).addTo(map.value)
  map.value.off('click')
  enableRealtime()
  capturarLecturaPeriodica() // <-- activar métricas
})
</script>

<style scoped>
/* --- FUENTES Y VARIABLES --- */
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700;800&family=Outfit:wght@300;500;700;800&family=Space+Grotesk:wght@400;700&display=swap');

:root {
  --bg-dark: #0c0f14;
  --bg-card: #161a20;
  --txt-main: #f5faff;
  --txt-muted: #98a3b3;
  --blue-main: #4a90e2;
  --blue-glow: 0 0 15px rgba(74, 144, 226, 0.6);
  --green-ok: #00e676;
  --orange-alert: #ff9100;
  --red-danger: #ff1744;
  --border: #262c35;
}

body {
  font-family: 'Inter', sans-serif;
  color: var(--txt-main);
  line-height: 1.6;
}

/* --- FONDO --- */
.hero-bg {
  position: fixed;
  inset: 0;
  z-index: -5;
  background:
    radial-gradient(circle at 20% 30%, #1a2330 0%, #0c0f14 55%),
    radial-gradient(circle at 80% 70%, #14212b 0%, #0c0f14 60%);
  animation: bgMove 10s alternate-reverse infinite ease-in-out;
}
@keyframes bgMove {
  0% {
    background-position: 0% 0%;
  }
  100% {
    background-position: 100% 100%;
  }
}

/* --- NAVBAR --- */
.navbar {
  padding: 1rem 2rem;
  background: rgba(22, 26, 32, 0.95);
  backdrop-filter: blur(14px);
  border-bottom: 1px solid var(--border);
  position: sticky;
  top: 0;
  z-index: 20;
}
.logo {
  font-size: 1.8rem;
  font-weight: 800;
  font-family: 'Outfit', sans-serif;
  color: var(--green-ok);
  text-shadow: 0 0 15px rgba(0, 230, 118, 0.3);
  margin: 0;
}

/* --- HERO --- */
.hero {
  text-align: center;
  padding: 3rem 1rem;
  width: min(1100px, 96%);
  margin: 2rem auto;
  border-radius: 20px;
  background: linear-gradient(135deg, #161a20cc, #1d232ccc);
  backdrop-filter: blur(25px);
  border: 1px solid var(--border);
  box-shadow: 0 10px 40px rgba(0, 0, 0, 0.45);
}
.hero h2 {
  font-size: clamp(2rem, 4vw, 3rem);
  font-family: 'Outfit', sans-serif;
  font-weight: 800;
  text-shadow: 0 0 8px rgba(245, 250, 255, 0.2);
}
.hero p {
  color: var(--txt-muted);
  font-size: 1.1rem;
}

/* --- TARJETAS --- */
.content-card {
  width: min(1100px, 96%);
  margin: 2rem auto;
  padding: 2rem;
  background: linear-gradient(135deg, #161a20e0, #1d232ce0);
  border-radius: 18px;
  border: 1px solid #2a303b;
  backdrop-filter: blur(18px);
  box-shadow: 0 14px 35px rgba(0, 0, 0, 0.45);
  transition: transform 0.3s ease;
}
.content-card:hover {
  transform: translateY(-3px);
  border-color: var(--blue-main);
}
.table-metricas {
  width: 100%;
  border-collapse: collapse;
  margin-top: 1rem;
  font-size: 0.9rem;
}

.table-metricas th,
.table-metricas td {
  padding: 8px 12px;
  border-bottom: 1px solid #2a303b;
}

.table-metricas th {
  background: #1c222c;
  text-align: left;
  color: #4a90e2;
}

.table-metricas tr:hover {
  background: #181d24;
}

.section-title {
  font-size: 1.6rem;
  margin-bottom: 1.5rem;
  font-family: 'Outfit';
  font-weight: 700;
}

/* --- MAPA --- */
.map {
  height: clamp(300px, 50vh, 650px);
  width: 100%;
  border-radius: 14px;
  border: 1px solid var(--border);
  box-shadow: 0 10px 30px rgba(0, 0, 0, 0.5);
  margin-bottom: 1.5rem;
}
.leaflet-container {
  background: #101419 !important;
}

/* --- BOTON WHATSAPP --- */
.btn-whatsapp {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 100%;
  max-width: 400px;
  margin: 1.5rem auto 0;
  padding: 16px 24px;
  border-radius: 12px;
  background: var(--blue-main);
  color: #fff;
  font-weight: 700;
  font-size: 1.1rem;
  border: none;
  cursor: pointer;
  box-shadow: var(--blue-glow);
  transition: 0.3s ease;
  font-family: 'Outfit';
}
.btn-whatsapp:hover {
  background: #357abd;
  transform: translateY(-2px);
  box-shadow: 0 0 20px rgba(74, 144, 226, 0.8);
}

/* --- ALERTAS Y BADGES --- */
.news-list {
  display: grid;
  gap: 1.5rem;
  grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
}
.alert-card {
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  padding: 1.5rem;
  background: #13171d;
  border-radius: 12px;
  border: 1px solid #2a303b;
  border-left: 5px solid;
  min-height: 180px;
  box-shadow: 0 4px 15px rgba(0, 0, 0, 0.3);
  transition: 0.25s ease;
}
.alert-card:hover {
  transform: translateY(-5px);
  background: #181d24;
}
.alert-card h4 {
  font-family: 'Space Grotesk';
  font-weight: 700;
  font-size: 1.1rem;
}
.alert-card p {
  color: var(--txt-muted);
  font-size: 0.95rem;
}

.alert-card .badge {
  align-self: flex-start;
  padding: 0.5rem 1rem;
  font-size: 0.8rem;
  border-radius: 50px;
  font-weight: 800;
  text-transform: uppercase;
  letter-spacing: 1px;
  margin-top: auto;
  color: #fff;
  border: 1px solid rgba(255, 255, 255, 0.1);
}

.type-ok {
  border-left-color: var(--green-ok);
}
.type-ok .badge {
  background-color: var(--green-ok);
  box-shadow: 0 0 15px rgba(0, 230, 118, 0.5);
}

.type-alert {
  border-left-color: var(--orange-alert);
}
.type-alert .badge {
  background-color: var(--orange-alert);
  box-shadow: 0 0 15px rgba(255, 145, 0, 0.5);
}

.type-danger {
  border-left-color: var(--red-danger);
}
.type-danger .badge {
  background-color: var(--red-danger);
  box-shadow: 0 0 15px rgba(255, 23, 68, 0.5);
}

/* --- FOOTER --- */
footer {
  margin-top: 3rem;
  padding: 2rem;
  text-align: center;
  color: var(--txt-muted);
  border-top: 1px solid var(--border);
  background: rgba(12, 15, 20, 0.8);
}
</style>
