#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

// --- CONFIGURAÇÃO DO ACCESS POINT ---
const char* ap_ssid = "Portal-SENAI"; // O nome da rede Wi-Fi que o ESP32 vai criar
const char* ap_password = "alunosenai"; // A senha para se conectar (mínimo 8 caracteres)

// --- OBJETOS GLOBAIS ---
WebServer server(80);
DNSServer dnsServer;

// --- PÁGINA HTML COMPLETA ARMAZENADA NA MEMÓRIA FLASH (PROGMEM) ---
// Usamos R"rawliteral(...)rawliteral" para colar o HTML diretamente sem precisar escapar caracteres.
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-br">
<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Bem-vindo ao SENAI - Localizador de Salas</title>
<style>
  /* CSS Unificado para toda a página */
  :root {
    --senai-blue: #00519e;
    --senai-orange: #ff0000;
    --senai-green: #009640;
    --senai-aqua: #00a99d;
    --text-color: #333;
    --background-light: #f4f4f4;
    --card-shadow: 0 4px 15px rgba(0, 0, 0, 0.1);
  }

  body {
    font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
    margin: 0;
    padding: 0;
    background-color: var(--background-light);
    color: var(--text-color);
  }

  .container {
    width: 80%;
    margin: auto;
    overflow: hidden;
  }

  /* --- Estilos do Header e Navegação --- */
  header {
    background: #fff;
    color: var(--senai-blue);
    padding: 20px 0;
    border-bottom: 5px solid var(--senai-orange);
    display: flex;
    justify-content: center;
    position: relative;
    z-index: 1000;
  }

  .header-content {
    width: 80%;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }

  header .logo img {
    width: 150px;
    display: block; 
  }

  header nav ul {
    padding: 0;
    margin: 0;
    list-style: none;
    display: flex; /* Adicionado para garantir alinhamento */
    align-items: center;
  }

  header nav ul li {
    display: inline;
    margin-left: 20px; /* Trocado para margin-left para espaçamento mais consistente */
  }
  
  header nav ul li:first-child {
      margin-left: 0;
  }

  header nav a {
    color: var(--senai-blue);
    text-decoration: none;
    font-weight: bold;
    font-size: 16px;
    cursor: pointer;
    transition: color 0.3s;
  }

  header nav a:hover {
    color: var(--senai-orange);
  }

  /* --- Itens do Menu Hambúrguer (Inicialmente escondidos no Desktop) --- */
  .menu-toggle {
    display: none;
  }

  .menu-icon {
    display: none;
    font-size: 28px;
    color: var(--senai-blue);
    cursor: pointer;
  }
  
  #showcase {
    min-height: 400px;
    background: url("https://www.sp.senai.br/images/share-facebook.jpg") no-repeat center
      center/cover;
    color: #fff;
    text-align: center;
    display: flex;
    flex-direction: column;
    justify-content: center;
    align-items: center;
    padding: 0 20px;
  }

  #showcase h1 {
    font-size: 55px;
    margin-bottom: 10px;
    text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.7);
  }

  #showcase p {
    font-size: 20px;
    text-shadow: 1px 1px 2px rgba(0, 0, 0, 0.7);
  }

  /* --- Estilos do Localizador de Salas --- */
  #localizador {
    padding: 40px 0;
    background-color: #fff;
  }
  .localizador-box {
    max-width: 700px;
    margin: auto;
    padding: 30px;
    border-radius: 12px;
    box-shadow: var(--card-shadow);
    text-align: center;
  }
  .localizador-box h1 {
    color: var(--senai-blue);
    margin-bottom: 10px;
  }
  .localizador-box p {
    font-size: 1.1em;
    color: #666;
    margin-bottom: 25px;
  }
  .input-group {
    display: flex;
    gap: 10px;
    margin-bottom: 25px;
  }
  input[type="text"],
  input[type="password"] {
    flex-grow: 1;
    padding: 15px;
    font-size: 1.1em;
    border: 2px solid #ddd;
    border-radius: 8px;
  }
  button {
    padding: 15px 25px;
    font-size: 1.1em;
    font-weight: bold;
    color: #fff;
    background-color: var(--senai-orange);
    border: none;
    border-radius: 8px;
    cursor: pointer;
    transition: background-color 0.3s;
  }
  button:hover {
    background-color: #d9641e;
  }
  #route-info {
    text-align: left;
    margin-top: 20px;
    display: none;
  }
  #route-info h2 {
    color: var(--senai-orange);
    border-bottom: 3px solid var(--senai-blue);
    padding-bottom: 10px;
  }
  #route-info .step {
    margin-bottom: 15px;
  }
  .map-container {
    width: 100%;
    height: 350px;
    border-radius: 8px;
    overflow: hidden;
    margin-top: 20px;
    border: 1px solid #ddd;
  }
  .map-container iframe {
    width: 100%;
    height: 100%;
    border: 0;
  }
  #error-message {
    color: #d8000c;
    background-color: #ffd2d2;
    padding: 10px;
    border-radius: 8px;
    display: none;
  }

  /* --- Estilos do Quadro de Avisos e Cursos --- */
  #main {
    padding: 20px 0;
  }
  .notice-board {
    background: #fff;
    padding: 20px;
    border-radius: 8px;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
  }
  .notice-board h2 {
    color: var(--senai-orange);
    border-bottom: 2px solid var(--senai-aqua);
    padding-bottom: 10px;
  }
  .notice {
    border-bottom: 1px solid #ddd;
    padding: 15px 0;
  }
  .notice:last-child {
    border-bottom: none;
  }
  .notice h3 {
    margin: 0 0 10px 0;
    color: var(--senai-blue);
  }
  .notice .date {
    font-size: 0.8em;
    color: #666;
    margin-bottom: 10px;
  }
  #courses {
    padding: 40px 0;
  }
  #courses h2 {
    text-align: center;
    color: var(--senai-blue);
    margin-bottom: 30px;
  }
  .course-gallery {
    display: flex;
    justify-content: space-around;
    flex-wrap: wrap;
    gap: 20px;
  }
  .course-card {
    background: #fff;
    width: 30%;
    min-width: 280px;
    margin-bottom: 20px;
    box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
    border-radius: 8px;
    overflow: hidden;
    text-align: center;
    display: flex;
    flex-direction: column;
  }
  .course-card img {
    width: 100%;
    height: 180px;
    object-fit: cover;
  }
  .course-card h3 {
    color: var(--senai-green);
    padding: 15px;
    flex-grow: 1;
  }
  .course-card p {
    padding: 0 15px 15px 15px;
  }
  .course-card a {
    display: inline-block;
    background: var(--senai-orange);
    color: #fff;
    padding: 10px 20px;
    text-decoration: none;
    border-radius: 5px;
    margin: 0 15px 15px;
    transition: background 0.3s;
  }
  .course-card a:hover {
    background: #d9641e;
  }
  footer {
    background: var(--senai-blue);
    color: #fff;
    text-align: center;
    padding: 20px 0;
    margin-top: 20px;
  }

  /* --- ESTILOS PARA LOGIN E PÁGINA DO ALUNO --- */
  .modal-overlay {
    position: fixed;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    background: rgba(0, 0, 0, 0.7);
    display: none;
    justify-content: center;
    align-items: center;
    z-index: 1000;
  }

  .login-modal {
    background: #fff;
    padding: 40px;
    border-radius: 12px;
    box-shadow: 0 5px 20px rgba(0, 0, 0, 0.3);
    text-align: center;
    max-width: 400px;
    width: 90%;
    position: relative;
  }
  .login-modal h2 {
    color: var(--senai-blue);
    margin-bottom: 20px;
  }
  .login-modal .input-group {
    flex-direction: column;
  }
  .login-modal input {
    width: calc(100% - 30px); /* Ajuste para o padding */
  }
  .login-modal button {
    width: 100%;
    margin-top: 10px;
  }
  .close-btn {
    position: absolute;
    top: 15px;
    right: 15px;
    font-size: 24px;
    color: #888;
    cursor: pointer;
  }
  #aluno-page {
    padding-top: 40px;
    display: none;
  }
  .aluno-header {
    text-align: center;
    margin-bottom: 30px;
  }
  .aluno-header h1 {
    color: var(--senai-blue);
  }
  .info-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
    gap: 30px;
  }
  .info-card {
    background: #fff;
    padding: 25px;
    border-radius: 12px;
    box-shadow: var(--card-shadow);
  }
  .info-card h2 {
    color: var(--senai-orange);
    border-bottom: 2px solid var(--senai-aqua);
    padding-bottom: 10px;
    margin-top: 0;
  }
  .info-card table {
    width: 100%;
    border-collapse: collapse;
    margin-top: 15px;
  }
  .info-card th,
  .info-card td {
    padding: 12px;
    text-align: left;
    border-bottom: 1px solid #ddd;
  }
  .info-card th {
    background-color: #f2f2f2;
  }
  .nota-boa { color: var(--senai-green); font-weight: bold; }
  .nota-media { color: #f0ad4e; font-weight: bold; }
  .nota-ruim { color: var(--senai-orange); font-weight: bold; }


  /* --- ESTILOS RESPONSIVOS PARA TODA A PÁGINA (MOBILE) --- */
  @media (max-width: 768px) {
    .container,
    .header-content {
      width: 90%;
    }
    
    .menu-icon {
      display: block;
      z-index: 1001;
    }
    
    header nav {
        order: 1; /* Garante que a nav fique depois do ícone */
    }

    header nav ul {
      position: absolute;
      top: 94px; /* Altura do header. Ajuste se o header mudar de tamanho */
      left: 0;
      width: 100%;
      display: flex;
      flex-direction: column;
      align-items: center;
      background-color: #ffffff;
      box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);
      transition: transform 0.3s ease-in-out;
      transform: translateY(-150%);
      z-index: 999;
      padding: 10px 0;
    }

    .menu-toggle:checked ~ nav ul {
      transform: translateY(0);
    }

    header nav ul li {
      display: block;
      margin: 0;
      width: 100%;
      text-align: center;
    }

    header nav a {
      padding: 20px 0;
      display: block;
      width: 100%;
      border-bottom: 1px solid #f0f0f0;
      font-size: 18px;
    }

    header nav ul li:last-child a {
      border-bottom: none;
    }
    
    #showcase h1 {
      font-size: 38px;
    }
    #showcase p {
      font-size: 18px;
    }
    .input-group {
      flex-direction: column;
    }
    .course-gallery {
      flex-direction: column;
      align-items: center;
    }
    .course-card {
      width: 95%;
      max-width: 350px;
    }
    .info-grid {
      grid-template-columns: 1fr;
    }
  }
</style>
</head>
<body>
    <div id="public-content">
        <header>
            <div class="header-content">
                <div class="logo">
                    <img src="src/senai.png" alt="Logo SENAI" />
                </div>

                <input type="checkbox" id="menu-toggle" class="menu-toggle" />
                <label for="menu-toggle" class="menu-icon">&#9776;</label>
                <nav>
                    <ul>
                        <li><a href="#showcase">Início</a></li>
                        <li><a href="#localizador">Localizador de Salas</a></li>
                        <li><a href="#main">Quadro de Avisos</a></li>
                        <li><a href="#courses">Cursos Disponíveis</a></li>
                        <li><a href="https://sp.senai.br/unidade/sbc/">Site Oficial</a></li>
                        <li><a id="login-btn">Entrar</a></li>
                    </ul>
                </nav>
            </div>
        </header>

        <section id="showcase">
            <div class="container">
                <h1>Bem-vindo ao Futuro, Bem-vindo ao SENAI!</h1>
                <p>Sua jornada para uma carreira de sucesso na indústria começa aqui.</p>
            </div>
        </section>

        <section id="localizador">
            <div class="localizador-box">
                <h1>Localizador de Salas</h1>
                <p>Digite o código da sua sala (Ex: A29, B1, D18) e clique em "Buscar".</p>
                <div class="input-group">
                    <input type="text" id="room-input" placeholder="Digite sua sala..." />
                    <button id="search-btn">Buscar</button>
                </div>
                <div id="error-message"></div>
                <div id="route-info">
                    <h2 id="route-title"></h2>
                    <div id="directions"></div>
                    <div class="map-container">
                        <img id="map-frame" src="" style="width: 100%; height: 100%" />
                    </div>
                </div>
            </div>
        </section>
        <div class="container" id="main">
            <section class="notice-board">
                <h2>Quadro de Avisos</h2>
                <div class="notice">
                    <h3>Início das Aulas - 2º Semestre de 2025</h3>
                    <p class="date">Publicado em: 01 de Setembro de 2025</p>
                    <p>As aulas do segundo semestre de 2025 terão início no dia 15 de Setembro. Fiquem atentos aos horários e salas.</p>
                </div>
                <div class="notice">
                    <h3>Palestra sobre Inovação na Indústria 4.0</h3>
                    <p class="date">Publicado em: 28 de Agosto de 2025</p>
                    <p>Participe da nossa palestra no auditório principal, no dia 20 de Setembro, às 19h.</p>
                </div>
                <div class="notice">
                    <h3>Se inscreva para a Olímpiadas do Conhecimento</h3>
                    <p class="date">Publicado em: 18 de Setembro de 2025</p>
                    <p>As inscrições para participar da Olímpiadas do conhecimento ocorrem anualmente, para mais informações procure a secretaria.</p>
                </div>
            </section>
        </div>
        <section id="courses">
            <div class="container">
                <h2>Cursos Disponíveis</h2>
                <div class="course-gallery">
                  <div class="course-card"><img src="src/Tecnico em Logística.jpg" alt="Curso em Logística"><h3>Técnico em Logística</h3><p>Este curso forma profissionais capacitados em planejamento, implementação e supervisão de projetos logísticos. possibilitados à organização de armazenagem, produção e transporte de bens.</p><a href="https://sp.senai.br/cursos/cursos-tecnicos/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Técnico em Manutenção de Máquinas Industriais.jpg" alt="Curso de Manutenção de Máquinas Industriais"><h3>Técnico em Manutenção de Máquinas Industriais</h3><p>O curso técnico em Manutenção de Máquinas Industriais forma profissionais para garantir o funcionamento e a manutenção de máquinas, equipamentos e instalações industriais. O foco é coordenar as atividades de manutenção mecânica.</p><a href="https://sp.senai.br/cursos/cursos-tecnicos/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Técnico em Mecânica.jpg" alt="Curso de Mecânica"><h3>Técnico em Mecânica</h3><p>O curso técnico em Mecânica forma profissionais para desenvolver competências em processos de manufatura subtrativa e aditiva, além de soluções tecnológicas na indústria metalmecânica. O foco também está na coordenação dos processos de produção industrial.</p><a href="https://sp.senai.br/cursos/cursos-tecnicos/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Técnico em Mecatrônica.jpg" alt="Curso de Mecatrônica"><h3>Técnico em Mecatrônica</h3><p>O curso técnico em mecatrônica visa formar profissionais capacitados a implementar e manter máquinas e equipamentos automatizados, bem como atuar no desenvolvimento de sistemas automatizados de manufatura.</p><a href="https://sp.senai.br/cursos/cursos-tecnicos/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Técnico em Móveis.jpg" alt="Curso de Móveis"><h3>Técnico em Móveis</h3><p>O curso técnico de Móveis forma profissionais para coordenar a fabricação e o desenvolvimento de móveis e esquadrias, controlando a qualidade e a conformidade dos processos e peças, conforme normas técnicas, de saúde, segurança, qualidade, ergonomia e ambientais.</p><a href="https/sp.senai.br/cursos/cursos-tecnicos/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Técnico em plástico.jpg" alt="Curso de plástico"><h3>Técnico em Plásticos</h3><p>O curso técnico de Plásticos forma profissionais para atuar no planejamento, desenvolvimento e controle da cadeia produtiva do plástico, conforme normas e legislações vigentes.</p><a href="https://sp.senai.br/cursos/cursos-tecnicos/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Técnico em Qualidade.jpg" alt="Curso de qualidade"><h3>Técnico em Qualidade</h3><p>O curso técnico em Qualidade forma profissionais para aplicar sistemas de gestão da qualidade e promover melhorias nos processos organizacionais, visando excelência e satisfação do cliente, com base em normas, legislações, tecnologias emergentes e princípios de ética, segurança e sustentabilidade.</p><a href="https://sp.senai.br/cursos/cursos-tecnicos/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Técnico em Química.jpg" alt="Curso de química"><h3>Técnico em Química</h3><p>O curso técnico de Química forma profissionais para atuar em processos químicos industriais e laboratoriais, seguindo normas técnicas, ambientais, de qualidade e de segurança no trabalho.</p><a href="https://sp.senai.br/cursos/cursos-tecnicos/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Assistente Administrativo.jpg" alt="Curso de Assistente Administrativo"><h3>Aprendiz em Assistente Administrativo</h3><p>Forma profissionais para apoiar atividades administrativas, organizacionais e de gestão, seguindo normas de qualidade e eficiência.</p><a href="https://sp.senai.br/cursos/aprendiz-senai/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Mecânico de Usinagem.jpg" alt="Curso de Mecânico de Usinagem"><h3>Aprendiz em Mecânico de Usinagem</h3><p>Forma profissionais para operar máquinas e fabricar peças mecânicas, seguindo normas de qualidade, segurança e meio ambiente.</p><a href="https://sp.senai.br/cursos/aprendiz-senai/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Eletricista de Manutenção Eletroeletrônica.jpg" alt="Curso de Eletricista de Manutenção Eletroeletrônica"><h3>Aprendiz em Eletricista de Manutenção Eletroeletrônica</h3><p>Forma profissionais para manter e reparar sistemas elétricos e eletroeletrônicos industriais, seguindo normas de segurança e qualidade.</p><a href="https://sp.senai.br/cursos/aprendiz-senai/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Ferramenteiro de Moldes para Plásticos.jpg" alt="Curso de Ferramenteiro de Moldes para Plásticos"><h3>Aprendiz em Ferramenteiro de Moldes para Plásticos</h3><p>Forma profissionais para confeccionar e ajustar moldes usados na fabricação de peças plásticas, conforme normas técnicas e de segurança.</p><a href="https://sp.senai.br/cursos/aprendiz-senai/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Mecânico de Manutenção.jpg" alt="Curso de Manutenção"><h3>Aprendiz em Manutenção</h3><p>Forma profissionais para realizar manutenção preventiva e corretiva em máquinas e equipamentos industriais, assegurando o funcionamento seguro e eficiente.</p><a href="httpss://sp.senai.br/cursos/aprendiz-senai/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                  <div class="course-card"><img src="src/Produtor Industrial de Móveis.jpg" alt="Curso de Produtor Industrial de Móveis"><h3>Aprendiz em Produtor Industrial de Móveis</h3><p>Forma profissionais para atuar na fabricação seriada de móveis, operando máquinas, controlando processos e garantindo qualidade e segurança na produção.</p><a href="https://sp.senai.br/cursos/aprendiz-senai/0?unidade=120&amp;turmasabertas=0">Saiba Mais</a></div>
                </div>
            </div>
        </section>
        <footer>
            <p>SENAI - Serviço Nacional de Aprendizagem Industrial © 2025</p>
        </footer>
    </div>

    <div class="modal-overlay" id="login-modal-overlay">
        <div class="login-modal">
            <span class="close-btn" id="close-login-btn">&times;</span>
            <h2>Portal do Aluno</h2>
            <div id="login-error" style="color: red; margin-bottom: 15px"></div>
            <div class="input-group">
                <input type="text" id="username-input" placeholder="Usuário" />
                <input type="password" id="password-input" placeholder="Senha" />
            </div>
            <button id="submit-login-btn">Entrar</button>
        </div>
    </div>
    <div id="aluno-page">
        <header>
            <div class="header-content">
                <div class="logo">
                    <img src="src/senai.png" alt="Logo SENAI" />
                </div>
                <nav>
                    <ul>
                        <li><a id="logout-btn">Sair</a></li>
                    </ul>
                </nav>
            </div>
        </header>
        <div class="container">
            <section class="aluno-header">
                <h1>Painel do Aluno</h1>
                <p>Seja bem-vindo(a), <strong id="aluno-nome">Nome do Aluno</strong>!</p>
            </section>
            <div class="info-grid">
                <div class="info-card">
                    <h2><img src="https://img.icons8.com/ios-filled/24/ff0000/calendar.png" style="vertical-align: middle; margin-right: 8px;"/>Cronograma de Aulas</h2>
                    <table id="cronograma-table">
                        <thead><tr><th>Dia</th><th>Horário</th><th>Matéria</th><th>Sala</th></tr></thead>
                        <tbody></tbody>
                    </table>
                </div>
                <div class="info-card">
                    <h2><img src="https://img.icons8.com/ios-filled/24/ff0000/report-card.png" style="vertical-align: middle; margin-right: 8px;"/>Notas e Faltas</h2>
                    <table id="notas-table">
                        <thead><tr><th>Matéria</th><th>Nota (0-100)</th><th>Faltas</th></tr></thead>
                        <tbody></tbody>
                    </table>
                </div>
            </div>
        </div>
        <footer>
            <p>SENAI - Serviço Nacional de Aprendizagem Industrial © 2025</p>
        </footer>
    </div>
<script>
  const dadosAlunos={caiomatosT4TC:{senha:"ilovegays",nome:"Caio Matos",cronograma:[{dia:"Seg",horario:"13:15-16:30",materia:"Eletrônica Digital",sala:"D36"},{dia:"Ter",horario:"13:15-16:30",materia:"Microcontroladores",sala:"D37"},{dia:"Qua",horario:"13:15-16:30",materia:"CLP Avançado",sala:"D36"},{dia:"Qui",horario:"13:15-16:30",materia:"Redes Industriais",sala:"D29"}],notas:[{materia:"Eletrônica Digital",nota:92,faltas:2},{materia:"Microcontroladores",nota:85,faltas:1},{materia:"CLP Avançado",nota:75,faltas:4},{materia:"Redes Industriais",nota:68,faltas:5}]},melissaluccasT2TC:{senha:"1234",nome:"Melissa Luccas",cronograma:[{dia:"Seg",horario:"13:15-16:30",materia:"Comandos Elétricos",sala:"Oficina Elétrica"},{dia:"Ter",horario:"13:15-16:30",materia:"Pneumática e Hidraúlica",sala:"Lab de Pneumática"},{dia:"Qua",horario:"13:15-16:30",materia:"Corrente Alternada",sala:"B1"}],notas:[{materia:"Comandos Elétricos",nota:88,faltas:0},{materia:"Pneumática e Hidraúlica",nota:95,faltas:1},{materia:"Corrente Alternada",nota:91,faltas:2}]}};
  const publicContent=document.getElementById("public-content"),alunoPage=document.getElementById("aluno-page"),loginModalOverlay=document.getElementById("login-modal-overlay"),loginBtn=document.getElementById("login-btn"),closeLoginBtn=document.getElementById("close-login-btn"),submitLoginBtn=document.getElementById("submit-login-btn"),logoutBtn=document.getElementById("logout-btn"),usernameInput=document.getElementById("username-input"),passwordInput=document.getElementById("password-input"),loginError=document.getElementById("login-error");function showLoginModal(){loginError.textContent="",loginModalOverlay.style.display="flex"}function hideLoginModal(){loginModalOverlay.style.display="none"}function showAlunoPage(e){publicContent.style.display="none",alunoPage.style.display="block",preencherDadosAluno(e)}function showPublicPage(){alunoPage.style.display="none",publicContent.style.display="block"}function preencherDadosAluno(e){document.getElementById("aluno-nome").textContent=e.nome;const o=document.querySelector("#cronograma-table tbody");o.innerHTML="",e.cronograma.forEach(e=>{const t=o.insertRow();t.innerHTML=`<td>${e.dia}</td><td>${e.horario}</td><td>${e.materia}</td><td>${e.sala}</td>`});const t=document.querySelector("#notas-table tbody");t.innerHTML="",e.notas.forEach(e=>{let o="";o=e.nota>=80?"nota-boa":e.nota>=60?"nota-media":"nota-ruim";const a=t.insertRow();a.innerHTML=`<td>${e.materia}</td><td class="${o}">${e.nota}</td><td>${e.faltas}</td>`})}function handleLogin(){const e=usernameInput.value,o=passwordInput.value,t=dadosAlunos[e];t&&t.senha===o?(hideLoginModal(),showAlunoPage(t)):loginError.textContent="Usuário ou senha inválidos."}function handleLogout(){usernameInput.value="",passwordInput.value="",showPublicPage()}loginBtn.addEventListener("click",showLoginModal),closeLoginBtn.addEventListener("click",hideLoginModal),submitLoginBtn.addEventListener("click",handleLogin),logoutBtn.addEventListener("click",handleLogout),loginModalOverlay.addEventListener("click",e=>{e.target===loginModalOverlay&&hideLoginModal()});const dadosDaEscola={A:{nome:"Bloco A - Oficinas de Metalmecânica e Marcenaria",instrucoesParaBloco:"Suba a rampa, siga em frente e desça as escadas. O bloco A estará bem na sua frente.",mapUrl:"src/mapaparablocoA.png",instrucoesDosAndares:{"0":'O Bloco A possui apenas um andar (térreo). Ao descer as escadas, você verá:\n                                  <ul>\n                                      <li><strong>Em frente:</strong> O setor de máquinas CNC e as salas <strong>A29</strong> e <strong>A30</strong>.</li>\n                                      <li><strong>À esquerda da porta de vidro:</strong> O laboratório de metrologia (<strong>A23</strong>) e, ao lado, as salas <strong>A22, A21 e A20</strong>.</li>\n                                      <li><strong>Corredor principal:</strong> Leva à oficina de metalmecânica, com os setores de Solda, Manutenção Mecânica e Usinagem (tornos e fresadoras).</li>\n                                      <li><strong>À direita, no fim da oficina:</strong> Acesso à oficina de marcenaria. Subindo a pequena rampa, ficam as salas <strong>A47</strong> e <strong>A46</strong>.</li>\n                                      <li><strong>À esquerda, no fim do pátio:</strong> Área de empilhadeiras.</li>\n                                  </ul>'}},B:{nome:"Bloco B - Coordenação, Secretaria e Química",instrucoesParaBloco:"Suba a rampa, siga em frente e vire à direita. O bloco B é o prédio com a porta de vidro à sua frente.",mapUrl:"src/mapaparablocoB.png",instrucoesDosAndares:{"0":"<strong>Térreo (Coordenação e Secretaria):</strong> Ao entrar pela porta de vidro, a <strong>Secretaria</strong> de todos os cursos fica à direita. A <strong>Coordenação</strong> fica à esquerda. A escada para o andar superior está ao lado da área da coordenação.","1":"<strong>1º Andar (Laboratórios de Química):</strong> Suba as escadas. O corredor possui os laboratórios na seguinte sequência (da esquerda para a direita): Lab Análise Instrumental, Lab Química Geral, Lab Microbiologia, Lab Operações Unitárias, Salas 1, 2 e 3, e a Sala de Reagentes (acesso restrito)."}},C:{nome:"Bloco C - Refeitório, Biblioteca e Auditório",instrucoesParaBloco:"Suba a rampa e siga em frente pelo pátio principal. O Bloco C é a grande estrutura à sua frente.",mapUrl:"src/mapaparablocoC.png",instrucoesDosAndares:{"0":"O Bloco C é térreo. Percorrendo pela lateral, você encontrará primeiro o <strong>Auditório</strong>, depois a <strong>Biblioteca</strong> e, por último, o <strong>Refeitório</strong>. Na parte de trás do bloco, localiza-se a sala <strong>C11</strong> (Tridimensional e GD&T)."}},D:{nome:"Bloco D - Oficina Elétrica, Labs e Salas de Aula",instrucoesParaBloco:"Suba a rampa, vire à direita e vá reto, passando pela entrada do Bloco B. Atravesse o refeitório (Bloco C), vire à direita novamente e suba as escadas. O Bloco D estará à sua frente.",mapUrl:"src/mapaparablocoD.png",instrucoesDosAndares:{"0":"<strong>Térreo:</strong> Ao entrar no bloco, vire à <strong>esquerda</strong> para encontrar o <strong>D18</strong> (Lab Tridimensional), <strong>D16</strong> (Lab de Hidráulica) e <strong>D15</strong> (Lab de Pneumática). Virando à <strong>direita</strong>, você encontrará o corredor com as salas de aula, da _D03_ até a _D18_ (do outro lado do corredor), incluindo as salas especiais _D06_ e _D06A_.","1":"_1º Andar:_ Subindo as escadas, vire à _direita_ para a _Oficina de Elétrica e Instalações Prediais_. Virando à _esquerda_, você encontrará as salas de tecnologia: _D29_ (Automação Predial), _D38_ (Projetos), _D37_ (Robótica) e _D36_ (CLP)."}}},roomInput=document.getElementById("room-input"),searchBtn=document.getElementById("search-btn"),routeInfo=document.getElementById("route-info"),errorMessage=document.getElementById("error-message"),routeTitle=document.getElementById("route-title"),directions=document.getElementById("directions"),mapFrame=document.getElementById("map-frame");function findRoom(){routeInfo.style.display="none",errorMessage.style.display="none";const e=roomInput.value;if(!e)return;const o=e.toUpperCase().replace(/[\s.-]/g,""),t=o.charAt(0),a=dadosDaEscola[t];let l;const n=o.substring(1);if("A"===t||"C"===t)l="0";else if("B"===t)l=n.charAt(0);else if("D"===t){const e=n.charAt(0);"0"===e||"1"===e?l="0":("2"===e||"3"===e)&&(l="1")}a&&a.instrucoesDosAndares[l]?(routeTitle.textContent=`Caminho para a sala ${e.toUpperCase()}`,directions.innerHTML=`\n                                  <div class="step"><strong>Passo 1: Vá para o ${a.nome}</strong><p>${a.instrucoesParaBloco}</p></div>\n                                  \n                                  <div class="step"><strong>Passo 2: Orientações no Bloco</strong><p>${a.instrucoesDosAndares[l]}</p></div>\n                                  <div class="step"><strong>Passo 3: Encontre sua sala</strong><p>Uma vez na área correta, procure pela placa com o número da sua sala.</p></div>\n                                `,mapFrame.src=a.mapUrl,routeInfo.style.display="block"):errorMessage.textContent=`Não foi possível encontrar a sala "${e}". Verifique se o código está correto (Ex: A29, B1, D18).`}searchBtn.addEventListener("click",findRoom),roomInput.addEventListener("keyup",function(e){"Enter"===e.key&&findRoom()});
</script>
</body>
</html>
)rawliteral";


void setup() {
  Serial.begin(115200);
  delay(1000);

  // --- INICIALIZAÇÃO DO MODO ACCESS POINT ---
  Serial.print("Configurando o Access Point...");
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP iniciado. Endereço IP: ");
  Serial.println(apIP);

  // --- CONFIGURAÇÃO DO PORTAL CATIVO ---
  dnsServer.start(53, "*", apIP);
  
  // Rota principal que serve o HTML armazenado em PROGMEM
  server.on("/", HTTP_GET, []() {
    // server.send_P é o comando para enviar dados da memória PROGMEM
    server.send_P(200, "text/html", INDEX_HTML);
  });

  // Se o cliente requisitar qualquer outra URL, ele será redirecionado para a página principal
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });

  // Inicia o servidor web
  server.begin();
  Serial.println("Servidor HTTP iniciado. Conecte-se a rede para abrir a pagina.");
}

void loop() {
  // Processa as requisições do servidor DNS (para o portal cativo)
  dnsServer.processNextRequest();
  // Processa as requisições do servidor web
  server.handleClient();
}