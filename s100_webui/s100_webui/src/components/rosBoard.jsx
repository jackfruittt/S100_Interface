import React from "react";

function ROSBoardPanel() {
  return (
    <div style={{ height: "100vh", width: "100%" }}>
      <iframe
        src="http://localhost:8000" // Replace with your ROSBoard URL
        style={{ width: "100%", height: "100%", border: "none" }}
        title="ROSBoard"
      />
    </div>
  );
}

export default ROSBoardPanel;
