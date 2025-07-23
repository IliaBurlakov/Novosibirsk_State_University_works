package minesweeper.model;

public class Cell {
    private boolean isMine;
    private boolean isOpened;
    private boolean isFlagged;
    private int minesAround;

    public Cell(boolean isMine, boolean isOpened, boolean isFlagged, int minesAround) {
        this.isMine = isMine;
        this.isOpened = isOpened;
        this.isFlagged = isFlagged;
        this.minesAround = minesAround;
    }

    public Cell() {
        this(false, false, false, 0);
    }

    public void reset() {
        this.isMine = false;
        this.isOpened = false;
        this.isFlagged = false;
        this.minesAround = 0;
    }

    public boolean isMine() { return isMine; }
    public boolean isOpened() { return isOpened; }
    public boolean isFlagged() { return isFlagged; }
    public int getMinesAround() { return minesAround; }

    public void setMine() throws IllegalStateException{
        if (this.isMine){
            throw new IllegalStateException("Mine is already set in this cell.");
        }
        if (this.isOpened){
            throw new IllegalStateException("Mine cannot be set at opened cell.");
        }
        this.isMine = true;
    }

    public void open () throws IllegalStateException{
        if (this.isOpened) {
            throw new IllegalStateException("Cell is already opened.");
        }
        if (this.isFlagged) {
            throw new IllegalStateException("Flagged cell cannot be opened.");
        }
        isOpened = true;
    }

    public void toggleFlag() {
        if (this.isOpened) {
            throw new IllegalStateException("Opened cell cannot be flagged.");
        }
        this.isFlagged = !this.isFlagged;
    }

    public void setMinesAround(int minesAround) throws IllegalArgumentException {
        if (minesAround < 0) {
            throw new IllegalArgumentException("Mines around count cannot be negative.");
        }
        if (minesAround > 8) {
            throw new IllegalArgumentException("Mines around count cannot be greater than 8.");
        }
        this.minesAround = minesAround;
    }
}
