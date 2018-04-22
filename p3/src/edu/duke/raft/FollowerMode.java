package edu.duke.raft;
import java.util.Timer;
import java.util.concurrent.ThreadLocalRandom; 

public class FollowerMode extends RaftMode {
  private Timer heartBeatTimer;

  public void go () {
    synchronized (mLock) {
      resetTimer();
      System.out.println ("S" + mID + "." + mConfig.getCurrentTerm() + ": switched to follower mode.");
    }
  }
  
  // @param candidate’s term
  // @param candidate requesting vote
  // @param index of candidate’s last log entry
  // @param term of candidate’s last log entry
  // @return 0, if server votes for candidate; otherwise, server's
  // current term
  public int requestVote (int candidateTerm,
			  int candidateID,
			  int lastLogIndex,
			  int lastLogTerm) {
   synchronized (mLock) {
      int term = mConfig.getCurrentTerm();
      int votedFor = mConfig.getVotedFor();
      // no log
      if(candidateTerm < term) {
	mConfig.setCurrentTerm(term, 0);
	return term;
      }
      else if(candidateTerm > term) {
	// additional shit
	mConfig.setCurrentTerm(candidateTerm, candidateID); 
	resetTimer(); // might have to reset with different period
	return 0;
      } else {
	if(votedFor == 0) {
	  // also additional shit
	  mConfig.setCurrentTerm(candidateTerm, candidateID);
	  resetTimer();
	  return 0;
	} else {
	  mConfig.setCurrentTerm(candidateTerm, 0);
	  return term;
	}
      }
    }
  }

  // @param leader’s term
  // @param current leader
  // @param index of log entry before entries to append
  // @param term of log entry before entries to append
  // @param entries to append (in order of 0 to append.length-1)
  // @param index of highest committed entry
  // @return 0, if server appended entries; otherwise, server's
  // current term
  public int appendEntries (int leaderTerm,
			    int leaderID,
			    int prevLogIndex,
			    int prevLogTerm,
			    Entry[] entries,
			    int leaderCommit) {
    synchronized (mLock) {
      int term = mConfig.getCurrentTerm ();
      int result = term;
      if(entries.length == 0) resetTimer();
      return result;
    }
  }  

  // @param id of the timer that timed out
  public void handleTimeout (int timerID) {
    synchronized (mLock) {
      System.out.println("HANDLE TIMEOUT TIMER ID: " + timerID);
      heartBeatTimer.cancel();
      RaftServerImpl.setMode(new CandidateMode());
    }
  }

  private void resetTimer() {
      int period = 0;
      if(mConfig.getTimeoutOverride() < 0)
	period = ThreadLocalRandom.current().nextInt(
	    RaftMode.ELECTION_TIMEOUT_MIN, RaftMode.ELECTION_TIMEOUT_MAX);
      else period = mConfig.getTimeoutOverride();
      heartBeatTimer = super.scheduleTimer(period, mID);
  }
}

