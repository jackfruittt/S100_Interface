import React, { useRef, useEffect } from "react";
import * as THREE from "three";
import { GLTFLoader } from "three/examples/jsm/loaders/GLTFLoader";
import { CSG } from "three-csg-ts";

const IMU3DShape = ({ roll, pitch, yaw }) => {
  const mountRef = useRef(null);
  const useGLB = false; 

  useEffect(() => {
    // Set up scene, camera, and renderer
    const scene = new THREE.Scene();
    scene.background = new THREE.Color(0xffffff);
    const camera = new THREE.PerspectiveCamera(75, 1, 0.1, 1000);
    const renderer = new THREE.WebGLRenderer({ antialias: true });

    renderer.setSize(400, 400);
    renderer.shadowMap.enabled = true; 
    mountRef.current.appendChild(renderer.domElement);

    // Add ambient light
    const ambientLight = new THREE.AmbientLight(0xffffff, 0.5);
    scene.add(ambientLight);

    // Add directional light
    const light = new THREE.DirectionalLight(0xffffff, 1);
    light.position.set(5, 5, 5);
    light.castShadow = true;
    scene.add(light);

    let model; 

    if (useGLB) {
      const loader = new GLTFLoader();
      loader.load(
        "/path/to/model.glb", // Implement later
        (gltf) => {
          model = gltf.scene;
          model.position.set(0, 0, 0);
          model.scale.set(1, 1, 1); 
          scene.add(model);
        },
        (xhr) => {
          console.log((xhr.loaded / xhr.total) * 100 + "% loaded");
        },
        (error) => {
          console.error(
            "An error occurred while loading the GLB model:",
            error
          );
        }
      );
    } else {
      const geometry = new THREE.BoxGeometry();
      const material = new THREE.MeshStandardMaterial({ color: 0x0077ff });
      model = new THREE.Mesh(geometry, material);
      model.castShadow = true;
      scene.add(model);
    }

    // Add hollow pipe using three-csg-ts
    const outerRadius = 1.5;
    const innerRadius = 1.4;
    const height = 5;

    const outerGeometry = new THREE.CylinderGeometry(outerRadius, outerRadius, height, 32);
    const innerGeometry = new THREE.CylinderGeometry(innerRadius, innerRadius, height, 32);
    const outerMesh = new THREE.Mesh(outerGeometry);
    const innerMesh = new THREE.Mesh(innerGeometry);

    const subtractedMesh = CSG.subtract(outerMesh, innerMesh);

    const pipeMaterial = new THREE.MeshStandardMaterial({
      color: 0xff8622,
      transparent: true,
      opacity: 0.5,
    });

    const pipe = new THREE.Mesh(subtractedMesh.geometry, pipeMaterial);
    pipe.rotation.x = Math.PI / 2;
    scene.add(pipe);

    // Add floor
    const floorGeometry = new THREE.PlaneGeometry(10, 10);
    const floorMaterial = new THREE.MeshStandardMaterial({ color: 0x9f9f9f });
    const floor = new THREE.Mesh(floorGeometry, floorMaterial);
    floor.rotation.x = -Math.PI / 2;
    floor.position.y = -2.5;
    floor.receiveShadow = true;
    scene.add(floor);

    // Set camera position
    camera.position.set(5, 5, 5);
    camera.lookAt(0, 0, 0);

    const animate = () => {
      requestAnimationFrame(animate);

      // Apply rotations based on IMU data
      
      if (model) {
        model.rotation.x = THREE.MathUtils.degToRad(pitch);
        model.rotation.y = THREE.MathUtils.degToRad(yaw);
        model.rotation.z = THREE.MathUtils.degToRad(roll);
      }
      
      /*
      //Automatic render for testing
      model.rotation.x += 0.02;
      model.rotation.y += 0.02;
      */
      renderer.render(scene, camera);
    };
    animate();

    // Cleanup
    return () => {
      mountRef.current.removeChild(renderer.domElement);
    };
  }, [pitch, yaw, roll, useGLB]);

  return <div ref={mountRef}></div>;
};

export default IMU3DShape;

