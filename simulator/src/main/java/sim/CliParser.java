package sim;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;

public class CliParser {
    private final Garland garland;
    private final OutputStream out;
    private final Runnable onStateChange;

    private final StringBuilder cmdBuf = new StringBuilder();
    private boolean waitPeriod = false;
    private String pendingSeq = "";
    private boolean interruptsEnabled = true;

    public CliParser(Garland garland, OutputStream out, Runnable onStateChange) {
        this.garland = garland;
        this.out = out;
        this.onStateChange = onStateChange;
    }

    public boolean isInterruptsEnabled() {
        return interruptsEnabled;
    }

    public void onByteReceived(int b) throws IOException {
        char ch = (char) b;

        if (ch == '\r' || ch == '\n') {
            if (!cmdBuf.isEmpty()) {
                String cmd = cmdBuf.toString().trim();
                cmdBuf.setLength(0);

                if (waitPeriod) {
                    processPeriod(cmd);
                } else if (!cmd.isEmpty()) {
                    processCommand(cmd);
                } else {
                    sendString("\r\n");
                }
                onStateChange.run();
            } else {
                sendString("\r\n");
            }
        } else if (ch == '\b' || ch == 0x7F) {
            if (!cmdBuf.isEmpty()) {
                cmdBuf.deleteCharAt(cmdBuf.length() - 1);
                sendString("\b \b");
            }
        } else if (ch >= ' ' && ch <= '~') {
            if (cmdBuf.length() < 31) {
                cmdBuf.append(ch);
                out.write(b); // Эхо символа обратно в терминал
                out.flush();
            }
        }
    }

    private void processCommand(String cmd) throws IOException {
        if (cmd.equals("set interrupts on")) {
            interruptsEnabled = true;
            sendString("\r\nOK\r\n");
        } else if (cmd.equals("set interrupts off")) {
            interruptsEnabled = false;
            sendString("\r\nOK\r\n");
        } else if (cmd.startsWith("new ")) {
            String seq = cmd.substring(4).trim();
            if (Garland.checkPattern(seq)) {
                pendingSeq = seq;
                waitPeriod = true;
                sendString("\r\nEnter period (1 - 200ms, 2 - 500ms, 3 - 1000ms):\r\n");
            } else {
                sendString("\r\nError: invalid sequence (use 2..8 chars of g, r, y, n)\r\n");
            }
        } else if (cmd.startsWith("set ")) {
            String arg = cmd.substring(4).trim();
            try {
                int num = Integer.parseInt(arg);
                if (garland.setMode(num)) {
                    sendString("\r\nOK\r\n");
                } else {
                    sendString("\r\nError: invalid mode number\r\n");
                }
            } catch (NumberFormatException e) {
                sendString("\r\nError: invalid mode number\r\n");
            }
        } else {
            sendString("\r\nError: unknown command\r\n");
        }
    }

    private void processPeriod(String input) throws IOException {
        int delay = switch (input) {
            case "1", "200" -> 200;
            case "2", "500" -> 500;
            case "3", "1000" -> 1000;
            default -> 0;
        };

        waitPeriod = false;

        if (delay > 0) {
            int modeNum = garland.addMode(pendingSeq, delay);
            sendString("\r\nSaved as mode " + modeNum + "\r\n");
        } else {
            sendString("\r\nError: invalid period\r\n");
        }
    }

    private void sendString(String s) throws IOException {
        out.write(s.getBytes(StandardCharsets.US_ASCII));
        out.flush();
    }
}