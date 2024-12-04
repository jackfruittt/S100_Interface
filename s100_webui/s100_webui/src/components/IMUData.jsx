import React from 'react';

function IMUData({ imuData }) {
  return (
    <div className="imu-data">
      <p>Roll: {imuData.roll.toFixed(2)}°</p>
      <p>Pitch: {imuData.pitch.toFixed(2)}°</p>
      <p>Yaw: {imuData.yaw.toFixed(2)}°</p>
    </div>
  );
}

export default IMUData;