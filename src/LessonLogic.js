import React from 'react';

class LessonLogic extends React.Component {
    constructor(props) {
        super(props);
    }

    render() {
        const LessonData = require('./data/french_data.json');
        const VerbNames = Object.keys(LessonData);

        return(
            <div>
                <p className='PromptText'>{LessonData[VerbNames[0]][0][2]}</p>
                <input type='text'/>
            </div>
        );
    }
}

export default LessonLogic;