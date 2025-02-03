import { defineConfig } from 'vite';
import react from '@vitejs/plugin-react';

export default defineConfig({
  plugins: [react()],
  build: {
    outDir: 'dist', // Output folder for bundled files
    assetsDir: 'assets', // Folder for static assets like images, fonts, etc.
    rollupOptions: {
      input: 'index.html', // Specify the root HTML file as the entry point
    },
  },
  server: {
    host: '0.0.0.0', // Allows access from other devices in your network
    port: 3000, // Default port for your UI
  },
  base: './', // Set base to relative path for correct resolution in the build
});





