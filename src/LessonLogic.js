import React from 'react';

class LessonLogic extends React.Component {
    constructor(props) {
        super(props);
    }

    render() {
        const LessonData = require('./data/french_data.json');
        const VerbNames = Object.keys(LessonData);

        // randomly generate the 10 verbs to be conjugated
        const Verbs = new Array(10);
        const Tenses = new Array(10);
        const Person = new Array(10);

        return(
            <div className='UIdiv'>
                <p className='PromptText'>{VerbNames[0]}</p>
                <input className='ResponseField' type='text'/>
            </div>
        );
    }
}

export default LessonLogic;