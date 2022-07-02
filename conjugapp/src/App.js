import './App.css';
import AppLogic from './AppLogic';

function App() {

  return (
    <div className="App">
      <header className="HeaderBar">
          <a href="./">
            <img src="gator_logo2.png" className="SmallLogo" />
          </a>
          <p className="AppTitle">ConjuGator</p>
      </header>
      <body>
        <AppLogic />
      </body>
      <footer className="FooterBar">

      </footer>
    </div>
  );
}

export default App;
