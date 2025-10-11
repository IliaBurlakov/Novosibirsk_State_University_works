package ru.nsu;

import java.io.IOException;
import java.net.*;
import java.util.*;

public class MulticastUDP {
    private static final int PORT = 3107;
    private static final int SEND_INTERVAL = 500;
    private static final int TIMEOUT = 5000;
    private static final int CLEANUP_INTERVAL = 1000;

    private volatile boolean running = true;
    private Map<String, Long> activeHosts = new HashMap<>();
    private String myId = UUID.randomUUID().toString();

    public static void main(String[] args) throws Exception {
        if (args.length != 1) {
            System.out.println("Usage: java MulticastUDP <multicast_ip>");
            return;
        }
        new MulticastUDP().start(args[0]);
    }

    private void start(String multicastIp) throws Exception {
        InetAddress group = validateAndGetAddress(multicastIp);
        MulticastSocket socket = new MulticastSocket(PORT);
        socket.setReuseAddress(true);
        joinMulticastGroup(socket, group);

        System.out.println("Started discovery on " + multicastIp);
        System.out.println("My ID: " + myId);
        System.out.println("Protocol: " + (group instanceof Inet4Address ? "IPv4" : "IPv6"));

        new Thread(() -> sendMessages(socket, group)).start(); // поток, отправляющий сообщения
        new Thread(this::periodicCleanup).start(); // поток, удаляющий неактивных членов группы

        receiveMessages(socket); // main поток получает сообщения и добавляет новых членов группы

        running = false;
        leaveMulticastGroup(socket, group);
        socket.close();
    }

    private InetAddress validateAndGetAddress(String multicastIp) {
        try {
            InetAddress group = InetAddress.getByName(multicastIp);

            if (!group.isMulticastAddress()) {
                System.err.println("ERROR: " + multicastIp + " is not a multicast address!");
                System.err.println("IPv4 multicast: 224.0.0.0 - 239.255.255.255");
                System.err.println("IPv6 multicast: ff00::/8");
                System.exit(1);
            }

            if (!(group instanceof Inet4Address) && !(group instanceof Inet6Address)) {
                System.err.println("ERROR: Unsupported address type");
                System.exit(1);
            }
            return group;

        } catch (UnknownHostException e) {
            System.err.println("ERROR: Invalid IP address: " + multicastIp);
            System.exit(1);
            return null;
        }
    }


    private void joinMulticastGroup(MulticastSocket socket, InetAddress group) throws IOException {
        NetworkInterface networkInterface = getPreferredNetworkInterface();

        if (networkInterface != null) {
            System.out.println("Using network interface: " + networkInterface.getDisplayName());
            socket.joinGroup(new InetSocketAddress(group, PORT), networkInterface);
        } else {
            System.out.println("No specific interface selected, using system default");
            if (group instanceof Inet6Address) {
                networkInterface = getAnySuitableInterface();
                if (networkInterface != null) {
                    socket.joinGroup(new InetSocketAddress(group, PORT), networkInterface);
                } else {
                    throw new IOException("No suitable network interface found for IPv6");
                }
            } else {
                socket.joinGroup(group);
            }
        }
    }

    private void leaveMulticastGroup(MulticastSocket socket, InetAddress group) throws IOException {
        NetworkInterface networkInterface = getPreferredNetworkInterface();

        if (networkInterface != null) {
            // Единый подход для выхода
            socket.leaveGroup(new InetSocketAddress(group, PORT), networkInterface);
        } else {
            if (group instanceof Inet6Address) {
                networkInterface = getAnySuitableInterface();
                if (networkInterface != null) {
                    socket.leaveGroup(new InetSocketAddress(group, PORT), networkInterface);
                }
            } else {
                socket.leaveGroup(group);
            }
        }
    }


    private NetworkInterface getPreferredNetworkInterface() {
        try {
            List<NetworkInterface> suitableInterfaces = new ArrayList<>();
            Enumeration<NetworkInterface> interfaces = NetworkInterface.getNetworkInterfaces();

            // Собираем ВСЕ подходящие интерфейсы
            while (interfaces.hasMoreElements()) {
                NetworkInterface ni = interfaces.nextElement();
                if (ni.isUp() && !ni.isLoopback() && ni.supportsMulticast()) {
                    suitableInterfaces.add(ni);
                }
            }

            if (suitableInterfaces.isEmpty()) {
                System.out.println("No suitable network interfaces found");
                return null;
            }

            // Пытаемся найти интерфейс с IPv6 (предпочтительно)
            for (NetworkInterface ni : suitableInterfaces) {
                Enumeration<InetAddress> addresses = ni.getInetAddresses();
                while (addresses.hasMoreElements()) {
                    if (addresses.nextElement() instanceof Inet6Address) {
                        return ni;
                    }
                }
            }

            // Если IPv6 не нашли - возвращаем первый подходящий
            return suitableInterfaces.get(0);

        } catch (SocketException e) {
            System.err.println("Error getting network interfaces: " + e.getMessage());
            return null;
        }
    }

    private NetworkInterface getAnySuitableInterface() {
        try {
            Enumeration<NetworkInterface> interfaces = NetworkInterface.getNetworkInterfaces();
            while (interfaces.hasMoreElements()) {
                NetworkInterface ni = interfaces.nextElement();
                if (ni.isUp() && !ni.isLoopback() && ni.supportsMulticast()) {
                    return ni;
                }
            }
        } catch (SocketException e) {
        }
        return null;
    }

    private void sendMessages(MulticastSocket socket, InetAddress group) {
        try {
            while (running) {
                String message = "ALIVE:" + myId;
                byte[] data = message.getBytes();
                socket.send(new DatagramPacket(data, data.length, group, PORT));
                Thread.sleep(SEND_INTERVAL);
            }
        } catch (Exception e) {
            if (running) e.printStackTrace();
        }
    }

    private void receiveMessages(MulticastSocket socket) throws Exception {
        byte[] buffer = new byte[1024];
        Set<String> lastActive = new HashSet<>();

        while (running) {
            DatagramPacket packet = new DatagramPacket(buffer, buffer.length);
            socket.receive(packet);

            String message = new String(packet.getData(), 0, packet.getLength());
            String senderIp = packet.getAddress().getHostAddress();

            if (message.startsWith("ALIVE:")) {
                String senderId = message.substring(6);

                if (!senderId.equals(myId)) {
                    activeHosts.put(senderIp, System.currentTimeMillis());
                }
            }
        }
    }

    private void periodicCleanup() {
        Set<String> lastActive = new HashSet<>();

        while (running) {
            try {
                cleanupInactive();
                checkAndPrintChanges(lastActive);
                Thread.sleep(CLEANUP_INTERVAL);
            } catch (InterruptedException e) {
                if (running) e.printStackTrace();
            }
        }
    }

    private void checkAndPrintChanges(Set<String> lastActive) {
        if (!activeHosts.keySet().equals(lastActive)) {
            System.out.println("Active hosts: " + activeHosts.keySet());
            lastActive.clear();
            lastActive.addAll(activeHosts.keySet());
        }
    }

    private void cleanupInactive() {
        long now = System.currentTimeMillis();
        activeHosts.entrySet().removeIf(entry -> now - entry.getValue() > TIMEOUT);
    }
}