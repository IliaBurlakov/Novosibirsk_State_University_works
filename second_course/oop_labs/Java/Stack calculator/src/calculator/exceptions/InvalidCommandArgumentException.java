package calculator.exceptions;

public class InvalidCommandArgumentException extends CalculatorException {
    public InvalidCommandArgumentException(String message) {
        super(message);
    }
}
