package minesweeper.model;

import javax.swing.*;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class GameModel {
    private int time;
    private int flagsLeft;
    private final int width;
    private final int height;
    private final int totalMines;
    private Cell[][] grid;
    private boolean gameOver;
    private boolean gameWon;
    private boolean firstMoveHappened;
    private String difficulty;

    public GameModel(int width, int height, int totalMines, String difficulty) throws IllegalArgumentException {
        if (width <= 0 || height <= 0 || totalMines <= 0)
            throw new IllegalArgumentException("Illegal arguments for the GameModel.");
        if (totalMines >= width * height)
            throw new IllegalArgumentException("Too many mines for this width and height.");
        this.width = width;
        this.height = height;
        this.totalMines = totalMines;
        this.flagsLeft = totalMines;
        this.firstMoveHappened = false;
        this.gameOver = false;
        this.gameWon = false;
        this.difficulty = difficulty;  // Устанавливаем сложность
        initializeField();
    }


    private void initializeField() {
        grid = new Cell[width][height];
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                grid[x][y] = new Cell();
            }
        }
    }

    public void setFirstMoveHappened(boolean firstMoveHappened) {
        this.firstMoveHappened = firstMoveHappened;
    }

    public boolean isGameOver() { return gameOver; }
    public boolean isGameWon() { return gameWon; }
    public int getTime() { return time; }
    public String getDifficulty() { return difficulty; }

    public int getFlagsLeft() { return this.flagsLeft; }
    public int getWidth() { return this.width; }
    public int getHeight() { return this.height; }
    public boolean isFirstMoveHappened() { return firstMoveHappened; }

    public Cell getCell(int x, int y) throws IndexOutOfBoundsException{
        if(!isValidPosition(x, y))
            throw new IndexOutOfBoundsException("Illegal cell position: (" + x + ", " + y + ")");
        return grid[x][y];
    }

    public void reset() {
        initializeField();
        gameOver = false;
        gameWon = false;
        firstMoveHappened = false;
        flagsLeft = totalMines;
        time = 0;
    }

    public void setTime(int time) {
        this.time = time;
    }

    static private ArrayList<CellPosition> getNeighboringCells(CellPosition cellPosition, int maxX, int maxY) {
        ArrayList<CellPosition> list = new ArrayList<>();
        CellPosition neighbourPos;
        for (int x = cellPosition.getX() - 1; x <= cellPosition.getX() + 1; x++) {
            for (int y = cellPosition.getY() - 1; y <= cellPosition.getY() + 1; y++) {
                if(x < 0 || y < 0 || x > maxX || y > maxY || (x == cellPosition.getX() && y == cellPosition.getY()))
                    continue;
                neighbourPos = new CellPosition(x, y);
                list.add(neighbourPos);
            }
        }
        return list;
    }

    private void calculateMinesAround() {
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                if (grid[x][y].isMine()) {
                    continue;
                }
                ArrayList<CellPosition> cellNeighbours= getNeighboringCells(new CellPosition(x, y), width - 1, height - 1);
                int minesAround = 0;
                for (CellPosition neighbourPos : cellNeighbours) {
                    if (grid[neighbourPos.getX()][neighbourPos.getY()].isMine())
                        minesAround++;
                }
                grid[x][y].setMinesAround(minesAround);
            }
        }
    }

    public void generateMines(int startX, int startY) throws IllegalArgumentException {
        List<CellPosition> availableForMinesCells = new ArrayList<>();
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                if (Math.abs(x - startX) > 1 || Math.abs(y - startY) > 1) {
                    availableForMinesCells.add(new CellPosition(x, y));
                }
            }
        }
        if (availableForMinesCells.size() < totalMines) {
            throw new IllegalArgumentException("Not enough cells to place" + totalMines + " mines.");
        }
        Collections.shuffle(availableForMinesCells);
        for (int i = 0; i < totalMines; i++) {
            CellPosition pos = availableForMinesCells.get(i);
            grid[pos.getX()][pos.getY()].setMine();
        }
        calculateMinesAround();
    }
    public void checkWin() {
        int safeCellsOpened = 0;
        for (Cell[] row : grid) {
            for (Cell cell : row) {
                if (cell.isOpened() && !cell.isMine()) {
                    safeCellsOpened++;
                }
            }
        }
        if (!gameOver && safeCellsOpened == width * height - totalMines && flagsLeft == 0)
            gameIsWon();
    }
    private void gameIsWon() {
        gameWon = true;
    }

    private void gameIsOver(){
        gameOver = true;
        for (Cell[] row : grid) {
            for (Cell cell : row) {
                if (cell.isMine()){
                    if (cell.isFlagged())
                        cell.toggleFlag();
                    if (!cell.isOpened())
                        cell.open();
                }
            }
        }
    }

    private boolean isValidPosition(int x, int y) { return x >= 0 && x < width && y >= 0 && y < height; }

    public void openCell(int x, int y) throws IndexOutOfBoundsException{
        if(!isValidPosition(x, y))
            throw new IndexOutOfBoundsException("Illegal cell position to open: (" + x + ", " + y + ")");
        if (grid[x][y].isOpened() || grid[x][y].isFlagged()) return;
        if (grid[x][y].isMine()) {
            gameIsOver();
            return;
        }
        if (gameOver || gameWon) return;
        grid[x][y].open();
        if (grid[x][y].getMinesAround() == 0)
            openNeighbors(x, y);
        checkWin();
    }
    private void openNeighbors(int x, int y) {
        ArrayList<CellPosition> neighbors = getNeighboringCells(new CellPosition(x, y), width - 1, height - 1);
        for (CellPosition pos : neighbors)
            openCell(pos.getX(), pos.getY());
    }

    public void toggleFlag(int x, int y) throws IllegalStateException, IndexOutOfBoundsException{
        if (gameOver || gameWon) return;

        if (!isValidPosition(x, y))
            throw new IndexOutOfBoundsException("Illegal cell position to flag: (" + x + ", " + y + ")");
        if (grid[x][y].isOpened())
            throw new IllegalStateException("Cant flag an opened cell");
        if(flagsLeft == 0)
            throw new IllegalStateException("There are no flags left.");

        if (grid[x][y].isFlagged()) {
            flagsLeft++;
            grid[x][y].toggleFlag();
            return;
        }
        flagsLeft--;
        grid[x][y].toggleFlag();
        checkWin();
    }
}
