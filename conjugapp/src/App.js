import './App.css';

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
        <button type="button" className="MainButton">
          Click Me!
        </button>
      </body>
    </div>
  );
}

export default App;
