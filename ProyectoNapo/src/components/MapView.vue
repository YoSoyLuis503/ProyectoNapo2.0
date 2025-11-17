<template>
  <div class="map-container">
    <div id="map" class="map"></div>

    <div class="toolbar">
      <button @click="locateUser">Ubicarme</button>
      <button @click="clearAll">Borrar alertas</button>
    </div>
  </div>
</template>

<script>
import { onMounted, ref } from "vue";
import L from "leaflet";
import "leaflet/dist/leaflet.css";

export default {
  name: "MapView",

  setup() {
    const map = ref(null);
    const storageKey = "alerts";

    const stateColors = {
      danger: "red",
      alert: "orange",
      ok: "green",
    };

    function loadAlerts() {
      const raw = localStorage.getItem(storageKey);
      try {
        return raw ? JSON.parse(raw) : [];
      } catch {
        return [];
      }
    }

    function saveAlerts(alerts) {
      localStorage.setItem(storageKey, JSON.stringify(alerts));
    }

    function makeIcon(color) {
      return L.divIcon({
        className: "alert-marker",
        html: `<span style="background:${color};width:16px;height:16px;display:block;border-radius:50%;border:2px solid white"></span>`,
        iconSize: [20, 20],
        iconAnchor: [10, 10],
      });
    }

    function addMarker(alert, save = true) {
      const marker = L.marker([alert.lat, alert.lng], {
        icon: makeIcon(stateColors[alert.state] || "gray"),
      }).addTo(map.value);

      marker.bindPopup(`
        <strong>${alert.state}</strong><br>
        ${alert.text || ""}<br>
        <button class="btn-delete" data-id="${alert.id}">Eliminar</button>
      `);

      marker.on("popupopen", () => {
        const btn = document.querySelector(`button[data-id='${alert.id}']`);
        if (btn) {
          btn.addEventListener("click", () => removeAlert(alert.id));
        }
      });

      if (save) {
        const alerts = loadAlerts();
        alerts.push(alert);
        saveAlerts(alerts);
      }
    }

    function removeAlert(id) {
      let alerts = loadAlerts();
      alerts = alerts.filter((a) => a.id !== id);
      saveAlerts(alerts);
      renderAll();
    }

    function clearAll() {
      localStorage.removeItem(storageKey);
      renderAll();
    }

    function renderAll() {
      // limpiar markers
      map.value.eachLayer((layer) => {
        if (layer instanceof L.Marker) map.value.removeLayer(layer);
      });

      const alerts = loadAlerts();
      alerts.forEach((a) => addMarker(a, false));
    }

    function locateUser() {
      map.value.locate({ setView: true, maxZoom: 14 });
    }

    onMounted(() => {
      // Inicializar mapa
      map.value = L.map("map").setView([0, 0], 2);

      L.tileLayer("https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png", {
        attribution: "© OpenStreetMap",
      }).addTo(map.value);

      // cargar markers iniciales
      renderAll();

      // clic para agregar alertas
      map.value.on("click", (e) => {
        const lat = e.latlng.lat;
        const lng = e.latlng.lng;

        const state = prompt(
          'Estado: "danger" (Peligro), "alert" (Precaución), "ok" (Tranquilo)',
          "danger"
        );
        if (!state || !["danger", "alert", "ok"].includes(state))
          return alert("Estado inválido");

        const text = prompt("Descripción (opcional)");
        const id = "a_" + Date.now();

        const alertObj = { id, lat, lng, state, text };
        addMarker(alertObj, true);
      });
    });

    return {
      locateUser,
      clearAll,
    };
  },
};
</script>

<style>
.map-container {
  width: 100%;
  height: 100%;
}
.map {
  height: 80vh;
  width: 100%;
}
.toolbar {
  margin-top: 10px;
  display: flex;
  gap: 10px;
}
.btn-delete {
  margin-top: 6px;
  padding: 4px 8px;
  background: #c0392b;
  color: white;
  border: none;
  border-radius: 4px;
}
</style>
