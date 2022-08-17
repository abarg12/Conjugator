import React from 'react';
import LessonLogic from './LessonLogic';

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
                    <button type="button" className="EndButton"
                            onClick={this.handleEndClick}>
                        End Lesson
                    </button>
                    <LessonLogic />
                </div>
            );
        } else {
            return (
                <div>
                    <h2>Learn French the brute force method!</h2>
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