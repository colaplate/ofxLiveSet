import React, { Component } from 'react';
import { connect } from "react-redux";

const mapStateToProps = state => {
    return { tracks: state.session.tracks };
};

class Track extends Component {
  render() {
    return (
      <div className="Track">
        track
      </div>
    );
  }
}

export default connect(mapStateToProps)(Track);
