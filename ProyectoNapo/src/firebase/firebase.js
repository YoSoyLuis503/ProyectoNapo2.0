// Import Firebase core
import { initializeApp } from 'firebase/app'
import { getFirestore } from 'firebase/firestore'

// TU CONFIGURACIÓN CORRECTA
const firebaseConfig = {
  apiKey: 'AIzaSyAaPeAYovM851vbHDplPGCbOXz2-mH6_HA',
  authDomain: 'proyectonapo-4d892.firebaseapp.com',
  projectId: 'proyectonapo-4d892',
  storageBucket: 'proyectonapo-4d892.appspot.com', // ← CORREGIDO
  messagingSenderId: '621281238664',
  appId: '1:621281238664:web:42c609cb3665acf8dd093d',
}

// Inicializar Firebase SIN ANALYTICS
const app = initializeApp(firebaseConfig)

// Exportar Firestore
export const db = getFirestore(app)
