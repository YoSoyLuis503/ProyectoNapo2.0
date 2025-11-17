// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { getAnalytics } from "firebase/analytics";
import { getFirestore } from "firebase/firestore";
// TODO: Add SDKs for Firebase products that you want to use
// https://firebase.google.com/docs/web/setup#available-libraries

// Your web app's Firebase configuration
// For Firebase JS SDK v7.20.0 and later, measurementId is optional
const firebaseConfig = {
  apiKey: "AIzaSyAaPeAYovM851vbHDplPGCbOXz2-mH6_HA",
  authDomain: "proyectonapo-4d892.firebaseapp.com",
  projectId: "proyectonapo-4d892",
  storageBucket: "proyectonapo-4d892.firebasestorage.app",
  messagingSenderId: "621281238664",
  appId: "1:621281238664:web:42c609cb3665acf8dd093d",
  measurementId: "G-EEZ1CXE071"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const analytics = getAnalytics(app);
export const db = getFirestore(app);