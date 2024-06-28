var socket = io("http://localhost:4000");

let Charts;

const renderData = (data) => {
  // console.log(data);
  var nhietdo = data.temp;
  var doam = data.hum;
  var gas_value = data.gas;
  var smoke_value = data.smoke;
  var created_at = data.created_at;

  var temperature1 = document.getElementById("temperature1");
  temperature1.innerHTML = `${nhietdo} ℃`;

  var humidity1 = document.getElementById("humidity1");
  humidity1.innerHTML = `${doam} %`;

  var gas1 = document.getElementById("gas1");
  gas1.innerHTML = `${gas_value} ppm`;

  var smoke1 = document.getElementById("smoke1");
  smoke1.innerHTML = `${smoke_value} ppm`;

  // socket.emit("Warning", "on");
  if(nhietdo >57 || gas_value >= 300) {
    socket.emit("Warning", "on");
    socket.emit("Led1", "on");
  }
  else {
    socket.emit("Warning", "off");
    socket.emit("Led1", "off");
  }


  if (Charts.data.labels.length >= 6) {
    Charts.data.datasets.forEach((el, index) => {
      el.data.shift();
      if (index == 0) {
        el.data.push(nhietdo);
      }
      else if (index == 1) {
        el.data.push(doam);
      }
      else {
        if (index == 2) {
          el.data.push(gas_value);
        }
      }
    });
    Charts.data.labels.shift();
  }

  Charts.data.labels.push(created_at);
  Charts.data.datasets.forEach((el, index) => {
    if (index == 0) {
      el.data.push(nhietdo);
    }
    else if (index == 1) {
      el.data.push(doam)
    }
    else {
      if (index == 2) {
        el.data.push(gas_value)
      }
    }
  }
  );
  Charts.update();
}


socket.on("sensorss", function (data) {
  if (data) {
    renderData(data);
  }

});


const data_chart = {
  labels: [],
  datasets: [
    {
      label: 'Temperature',
      data: [],
      borderColor: "red",
      backgroundColor: "red",
      yAxisID: 'y',
    },
    {
      label: 'Humidity',
      data: [],
      borderColor: "blue",
      backgroundColor: "blue",
      yAxisID: 'y',
    }, {
      label: 'Gas',
      data: [],
      borderColor: "rgb(161, 206, 64)",
      backgroundColor: "rgb(161, 206, 64)",
      yAxisID: 'y1',
    }
  ]
}

const config = {
  type: 'line',
  data: data_chart,
  options: {
    responsive: true,
    plugins: {
      title: {
        display: true,
        text: 'Line Chart - Humidity, Temperature and Gas_value'
      },
    },
    interaction: {
      intersect: false,
    },
    scales: {
      y: {
        display: true,
        max: 100,
        title: {
          display: true,
          text: 'Temperature and Humidity'
        },
        position: "left",
      },
      y1: {
        display: true,
        max: 2000,
        title: {
          display: true,
          text: 'Gas'
        },
        position: "right",
      },
    }
  },
};
Charts = new Chart("chart", config);

