import React from 'react';

class AppLogic extends React.Component {
    constructor(props) {
        super(props);
        this.handleStartClick = this.handleStartClick.bind(this);
        this.handleEndClick = this.handleEndClick.bind(this);
        this.state = {
            LessonInProgress: false,
        };
    }

    handleStartClick() {
        this.setState({LessonInProgress: true});
    }

    handleEndClick() {
        this.setState({LessonInProgress: false});
    }

    render() {
        const in_prog = this.state.LessonInProgress;

        if (in_prog) {
            return (
                <div>
                    <button type="button" className="MainButton"
                            onClick={this.handleEndClick}>
                        End Lesson
                    </button>
                    <p>Test test test</p>
                </div>
            );
        } else {
            return (
                <div>
                    <button type="button" className="MainButton"
                            onClick={this.handleStartClick}>
                        Start practicing! 
                    </button>
                </div>
            );
        }
    }
}

export default AppLogic;