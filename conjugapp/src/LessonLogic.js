import React from 'react';

class LessonLogic extends React.Component {
    constructor(props) {
        super(props);
    }

    render() {
        const LessonData = require('./data/french_data.json');

        return(
            <div>
                <p>{LessonData.verbs.être.indicative.present[0]}</p>
            </div>
        );
    }
}

export default LessonLogic;