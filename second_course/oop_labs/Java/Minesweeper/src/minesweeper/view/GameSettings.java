package minesweeper.view;

public class GameSettings {
    private int width;
    private int height;
    private int mines;
    private String difficulty;
    public GameSettings() {
        setDifficulty("easy");
        difficulty = "easy";//default
    }

    public void setDifficulty(String difficulty) {
        switch (difficulty.toLowerCase()) {
            case "easy":
                width = 9;
                height = 9;
                mines = 10;
                difficulty = "easy";
                break;
            case "medium":
                width = 16;
                height = 16;
                mines = 40;
                difficulty = "medium";
                break;
            case "hard":
                width = 16;
                height = 30;
                mines = 99;
                difficulty = "hard";
                break;
            default:
                throw new IllegalArgumentException("Unknown difficulty level: " + difficulty);
        }
    }

    public void setCustom(int width, int height, double minesPercentage) {
        this.width = width;
        this.height = height;
        int totalCells = width * height;
        difficulty = "custom";
        this.mines = (int) Math.max(1, Math.min(totalCells * minesPercentage, totalCells - 1)); // 1 <= mines <= totalCells - 1
    }

    // Геттеры
    public int getWidth() { return width; }
    public int getHeight() { return height; }
    public int getMines() { return mines; }

    public String getDifficulty() {
        return difficulty;
    }

}
