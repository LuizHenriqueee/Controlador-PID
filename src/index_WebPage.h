const char WEB_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="pt-br">
<!--Comentários-->
<head>
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
  <meta name="description" content="">
  <meta name="author" content="">
  <meta name="viewport" content="width=device-width, initial-scale=1">

  <!--Incluindo Bibliotecs JS & Arquivos CSS-->  
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.0/css/bootstrap.min.css">
  <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.4.0/jquery.min.js"></script>
  <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.4.0/js/bootstrap.min.js"></script>
  <script src = "https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.3/Chart.min.js"></script>
  
  <meta charset="UTF-8"/>
  <title>Bancada PID</title>

  <style>
    canvas{
      -moz-user-select: none;
      -webkit-user-select: none;
      -ms-user-select: none;
  }
    
  /*Formatação das Tabelas*/
  #table {
      font-family: "Trebuchet MS", Arial, Helvetica, sans-serif;
      border-collapse: collapse;
      width: 100%;
  }
  #table td, #table th {
      border: 1px solid #ddd;
      padding: 8px;
      font-weight: bold;
  } 
  #table tr:nth-child(even){background-color: #f2f2f2;}
  #table tr:hover {background-color: #ddd;}
  #table th {
      padding-top: 12px;
      padding-bottom: -30px;
      text-align: left;
      background-color: #4CAF50;
      color: white;
  }
  
  /*Botões*/
  .button {
    display: inline-block;
    padding: 8px 8px;
    font-size: 24px;
    cursor: pointer;
    text-align: center;
    text-decoration: none;
    outline: none;
    color: #fff;
    background-color: #4CAF50;
    border: none;
    border-radius: 15px;
    box-shadow: 0 9px #999;
  }
  .button:hover {background-color: #3e8e41}
  .button:active {
    background-color: #3e8e41;
    box-shadow: 0 5px #666;
    transform: translateY(4px);
  }
  /*Div de resultado não estável*/
  #divMainNotStable {
    float: left;
    position: absolute;
    padding-bottom: 100px;
    left: 50%;
  }
  #systemNotStable{
    display: inline-block;
    width: 250px;
    height: 50px;
    padding: 10px;
    margin: 20px;
    color:#fff;
    font-size: 15pt;
    text-align: center;
    float: left;
    position: relative;
    left: -50%;
    border-radius: 15px;
    background-color: red;
  }
</style>

</head>

<body id="page-top">
  <nav class="navbar navbar-inverse">
    <div class="container-fluid">
      <div class="navbar-header">
        <a class="navbar-brand" href="#">Bancada PID</a>
      </div>
    </div>
  </nav>
  <!--Tabela de Entradas-->
  <div id="inputData">
    <div style="text-align:center;"><h1>Dados de Entrada</h1></div>
    <table id="table">
      <tr><th>Kp</th><th>Ki</th><th>Kd</th><th>Sinal de Referência</th><th>Status</th></tr>
      <tr><td id="Kp">0</td><td id="Ki">0</td><td id="Kd">0</td><td id="refSignal">0</td><td id="status">Estável</td></tr>
    </table>
  </div>

  <br/>

  <!--Gráfico-->
  <div style="text-align:center;"><b>Gráfico do Controlador</b><br></div>
  <div class="chart-container" position: relative; height:350px; width:100%">
      <canvas id="Chart" width="400" height="400"></canvas>
  </div>
  
  <!--Botão Stop-->
  <div id="divBtnStop" style="text-align:center;"> 
    <button class="button" type="button" id="btnStop" style="width: 180px; height: 50px; background-color: red;" 
    onclick="sendRequestStop()">Stop!</button>
  </div>

  <!--Resultado: Sistema Estável-->
  <div id="systemStable">
    <div style="text-align:center;"><h2>Parâmetros de Desempenho</h2><br></div>
    <table id="table">
      <tr>
        <th>Tempo de Subida/Descida</th><th>Tempo de Pico</th><th>Overshoot</th><th>Tempo de Acomodação</th><th>Erro Regime</th>
      </tr>
      <tr>
          <td id="tSubida">0</td>
          <td id="tPico">0</td>
          <td id="ovrShoot">0</td>
          <td id="tAcomodacao">0</td>
          <td id="erRegime">0</td>
      </tr>
    </table>
  </div>
  <!--Resultado: Sistema Não Estável-->
  <div id="divMainNotStable">
    <div id="systemNotStable"><b>Sistema Não Estável !</b><br></div>
  </div>
  
  <br/>
  <br/>
  <br/>
  <br/>
  <!--Botão de Gerar Relatório-->
  <div id="divReport" style="text-align:center;">
    <button class="button" id="btnReport">Gerar Relatório</button>
  </div>


  <script>
  var txUpadate = 100;//Taxa de atualização
  var distanceValues = [];
  var dataSamples = [];
  var referenceGraphic = [];
  var lastStatus = "Inicializando";
  var dist_floor = [35.0, 160.0, 286.0, 412.0, 535.0];
  var sample = 0;
  var reset;

  function loadGraphic(){
      var ctx = document.getElementById("Chart").getContext('2d');
      var Graphic = new Chart(ctx, {
          type: 'line',
          data: {
              labels: dataSamples,  
              datasets: [
                  {
                    label: "Distância",
                    fill: false,
                    borderColor: 'rgba(0,0,200,.85)',
                    backgroundColor: 'transparent',
                    data: distanceValues, 
                    borderWidth: 3,
                    pointRadius: 0,
                  },
                  {
                    label:"Sinal de Referência",
                    borderColor: 'red',
                    backgroundColor: 'transparent',
                    data: referenceGraphic,  
                    borderWidth: 2,
                    pointRadius: 0,
                  }
              ]
          },
          options: {
              title:{
                display: true,
                text: ""
              },
              maintainAspectRatio: false,
              animation: false,
              elements: {
              line: {
                      tension: 0.5 //Suavização das curvas
                  }
              },
              scales: {
                      xAxes:[{
                        display: true,
                        scaleLabel:{
                          display: true,
                          fontColor: 'rgba(0,0,200,.85)',
                          fontSize:12,
                          fontStyle: "bold",
                          labelString:'Tempo (ms)'
                        }
                      }],
                      yAxes:[{
                        display: true,
                        gridLines: {
                          drawBorder: false,
                        },
                        ticks: {
                          min: -100,
                          max: 1100,
                          stepSize: 100
                        },
                        scaleLabel:{
                          display: true,
                          fontColor: 'rgba(0,0,200,.85)',
                          fontSize:12,
                          fontStyle: "bold",
                          labelString:'Distância (mm)'
                        }
                      }]
              }
          }
      });
  }

  //Executa as funções após página ser carregada
  window.onload = function() {
    loadGraphic();
  };
  
  //Chamada de getUrlData() de acordo com a taxa de atualização
  setInterval(function() {
    getUrlData();
  }, txUpadate);

  //Gera o relatório do sistema
  function generateReport(){
    window.print();
  }
  
  var report = document.getElementById("btnReport");
  
  function sendRequestStop() {
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "setSTOP?STOPstate="+1, true); 
    xhttp.send();
  }

  function getUrlData() {
    var xhttp = new XMLHttpRequest();
    //Função que tratará a resposta do servidor
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        //Pegando JSON da URL
        var jsonData = this.responseText;
        var data = JSON.parse(jsonData);
        switch (data.SystemStatus){
          case "Inicializando":
            reset = true; 
            //Valores de Entrada
            document.getElementById("Kp").innerHTML = parseFloat(data.Kp);
            document.getElementById("Kd").innerHTML = parseFloat(data.Kd);
            document.getElementById("Ki").innerHTML = parseFloat(data.Ki);
            document.getElementById("status").innerHTML = data.SystemStatus + ": Aguardando entradas...";//Status do Sistema
            
            //Setando visibilidade das Divs de Resultado
            document.getElementById("divBtnStop").style.display = 'none';
            if(lastStatus == "Estavel" || lastStatus == "Nao_Estavel"){
              if(lastStatus == "Estavel"){
                document.getElementById("systemStable").style.display = 'block';  
                document.getElementById("divMainNotStable").style.display = 'none';
              } 
              if(lastStatus == "Nao_Estavel"){
                  document.getElementById("divMainNotStable").style.display = 'block';
                  document.getElementById("systemStable").style.display = 'none'; 
              }             
              document.getElementById("divReport").style.display = 'block';
            }
            else{
              document.getElementById("systemStable").style.display = 'none'; 
              document.getElementById("divMainNotStable").style.display = 'none';
              document.getElementById("divReport").style.display = 'none';
            }
                      
            loadGraphic();
            break;  
          case "Estabilizando":
            if(reset){
              //Reinicialização do sistema
              //Limpando Dados Gráfico
              distanceValues = [];
              dataSamples  = [0];
              referenceGraphic = [dist_floor[parseInt(data.ReferenceSignal) - 1]];
              sample = 0;
         
              //Formato Sinal de referência: 1000mm | 1º Andar 
              document.getElementById("refSignal").innerHTML = dist_floor[parseInt(data.ReferenceSignal) - 1] + "mm | " +  data.ReferenceSignal + "º Andar" ;//Sinal de Refência
            }
            //Inserindo valores novos dados nos vetores de dados.
            distanceValues.push(data.Distance);
            dataSamples.push(++sample);
            document.getElementById("status").innerHTML = data.SystemStatus;//Status do Sistema
            referenceGraphic.push(dist_floor[parseInt(data.ReferenceSignal) - 1]);
            loadGraphic();//Atualização do Gráfico

            //Setando visibilidade das Divs de Resultado
            document.getElementById("systemStable").style.display = 'none';
            document.getElementById("divMainNotStable").style.display = 'none';
            document.getElementById("systemStable").style.display = 'none';
            document.getElementById("divReport").style.display = 'none';
            document.getElementById("divBtnStop").style.display = 'block';
            reset = false;
            sample++;
    
            break;
          case "Estavel":
            //Mensagem de Sucesso + Parêmetros de Desempenho...
            document.getElementById("tSubida").innerHTML = data.UpTime;
            document.getElementById("tPico").innerHTML = data.PeakTime;
            document.getElementById("ovrShoot").innerHTML = data.Overshoot;
            document.getElementById("tAcomodacao").innerHTML = data.AccommodationTime;
            document.getElementById("erRegime").innerHTML = data.RegimeError;
            document.getElementById("status").innerHTML = data.SystemStatus;//Status do Sistema
            //Setando visibilidade das Divs de Resultado
            document.getElementById("systemStable").style.display = 'block';
            document.getElementById("divMainNotStable").style.display = 'none';
            document.getElementById("divReport").style.display = 'block';
            //Gerando Relatório
            report.addEventListener("click", generateReport);
            lastStatus = "Estavel";
            break;
          case "Nao_Estavel":
            //Mensagem de Falha....
            document.getElementById("status").innerHTML = data.SystemStatus;//Status do Sistema
            //Setando visibilidades das Divs de Resultado
            document.getElementById("divMainNotStable").style.display = 'block';
            document.getElementById("systemStable").style.display = 'none';
            document.getElementById("divReport").style.display = 'block';
            //Gerando Relatório
            report.addEventListener("click", generateReport);
            lastStatus = "Nao_Estavel";
            break;
        }
      }
    };
    xhttp.open("GET", "readRequest", true);
    xhttp.send();
  }
  </script>
</body>
</html>
)=====";
