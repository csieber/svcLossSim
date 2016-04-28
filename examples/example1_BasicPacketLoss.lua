--
-- Pkt Loss Model Sample File
-- sieber@informatik.uni-wuerzburg.de
--

--
-- [============================== Global Variables ==============================]
--

pkt_counter = 0;

--
-- [============================ Function 'initialize' ===========================]
--
-- function initialise(..)
--
-- initialise() is called when the sim application starts.
--
-- = Return =
--
-- -1: Failed with unspecified error.
-- 	0: Success.
--
function initialise()
	
	math.randomseed( os.time() );
	
	return 0;
end


--
-- [============================ Function 'drop' ===========================]
--
--
-- function drop(..)
--
-- This function is called to determine how a specific packet
-- should be handled.
--
-- Parameters:
--   frame_nr
--		The frame the packet belongs to.
--
--   nal_nr
--		Number of the NAL in the stream.
--
--   nal_type
--		The type of the NAL unit. See H.264/Annex.G for details.
--
--   nal_size
--		Size of the whole NAL unit.
--
--   D
--		Dependency layer the NAL unit belongs to. (Resolution)
--
--   T
--		Temorary layer. (frame rate)
--
--   Q
--		Quality layer.
--
--   pkt_size
--		Packet size of this chunk of the NAL unit
--
--   pkt_n
--		Packet is the n-part of the nal unit.
--
--   pkt_c
--		At whole the NAL unit is splitted into pkt_c packets.
--
--
-- = Return =
--
--  -1: Invalid input parameters.
--   0: Output this packet.
--   1: Silently drop this packet.
--   2: Drop this packet, output zeros with the same size instead.
--   3: Drop NAL data, but keep NAL header. (only applicable for the first packet)
--	 4: Zero NAL data, but keep NAL header. (only applicable for the first packet)

function drop(frame_nr, nal_nr, nal_type, nal_size, D, T, Q, pkt_size, pkt_n, pkt_c)

	pkt_counter = pkt_counter + 1;
	
	--
	-- Random 1% packet loss, but we keep the NAL separators
	-- and NAL headers.
	--
	local rand = math.random(1, 100);
	
	if rand == 1 then
		return 4;
	else
		return 0;
	end
end

--
-- [============================ Function 'finished' ===========================]
--
--
-- function finished(..)
--
-- This function is called when the end of the stream is reached.
--
-- Parameters:
--   (none so far)
--
-- = Return =
--
--  (none so far)
--

function finished()
	
	print('LUA: Finished(). drop() was called ' .. pkt_counter .. ' times.');

	return 0;
end
