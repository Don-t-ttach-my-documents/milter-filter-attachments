 -- Echo that the test is starting
       mt.echo("*** begin test")
       -- start the filter
       mt.startfilter("../myFilter", "-p", "inet:8800@localhost")
       mt.echo("filter started")
       mt.sleep(2)

       -- try to connect to it
       conn = mt.connect("inet:8800@localhost")
       if conn == nil then
       	     mt.echo("Connexion failed")
               error "mt.connect() failed"
       end

       -- send connection information
       -- mt.negotiate() is called implicitly
       if mt.conninfo(conn, "localhost", "127.0.0.1") ~= nil then
               mt.echo("mt.conninfo() failed")
            error "mt.conninfo() failed"
       end
       if mt.getreply(conn) ~= SMFIR_CONTINUE then
               mt.echo("mt.conninfo unexpected reply")
            error "mt.conninfo() unexpected reply"
       end

       -- send envelope macros and sender data
       -- mt.helo() is called implicitly
       mt.macro(conn, SMFIC_MAIL, "i", "test-id")
       if mt.mailfrom(conn, "user@example.com") ~= nil then
          mt.echo("mt.mailfrom() failed")
            error "mt.mailfrom() failed"
       end
       if mt.getreply(conn) ~= SMFIR_CONTINUE then
               mt.echo ("mt.mailfrom unexcpected reply")
            error "mt.mailfrom() unexpected reply"
       end

       -- send headers
       -- mt.rcptto() is called implicitly
       if mt.header(conn, "From", "user@example.com") ~= nil then
               mt.echo('mt.header(From) failed')
            error "mt.header(From) failed"
       end
       if mt.getreply(conn) ~= SMFIR_CONTINUE then
               mt.echo("mt.header(From) unexpected reply")
            error "mt.header(From) unexpected reply"
       end
       if mt.header(conn, "Date", "Tue, 22 Dec 2009 13:04:12 -0800") ~= nil then
          mt.echo("mt.header(Date) failed")
            error "mt.header(Date) failed"
       end
       if mt.getreply(conn) ~= SMFIR_CONTINUE then
               mt.echo("mt.header(Date) unexcpected reply")
            error "mt.header(Date) unexpected reply"
       end
       if mt.header(conn, "Subject", "Signing test") ~= nil then
               error "mt.header(Subject) failed"
            error "mt.header(Subject) failed"
       end
       if mt.getreply(conn) ~= SMFIR_CONTINUE then
          mt.echo("mt.header(Subject) unexpected reply")
            error "mt.header(Subject) unexpected reply"
       end
       -- send EOH
       if mt.eoh(conn) ~= nil then
               mt.echo ("mt.eoh() failed")
            error "mt.eoh() failed"
       end
       if mt.getreply(conn) ~= SMFIR_CONTINUE then
               mt.echo("mt.eoh() unexpected reply")
            error "mt.eoh() unexpected reply"
       end

       -- send body
      --  if mt.bodystring(conn, "This is a test!\nA pretty test!\n") ~= nil then
      --          mt.echo("mt.bodystring() failed")
      --       error "mt.bodystring() failed"
      --  end

       if mt.bodyfile(conn, "mime_body.txt") ~= nil then
         mt.echo("mt.bodyfile() failed")
         error "mt.bodyfile() failed"
       end

       if mt.getreply(conn) ~= SMFIR_CONTINUE then
               mt.echo("mt.bodystring() unexpected reply")
            error "mt.bodystring() unexpected reply"
       end
       -- end of message; let the filter react
       if mt.eom(conn) ~= nil then
               mt.echo("mt.eom() failed")
            error "mt.eom() failed"
       end
     --   if mt.getreply(conn) ~= SMFIR_ACCEPT then
     --           mt.echo("mt.eom() unexpected reply")
     --        error "mt.eom() unexpected reply"
     --   end

       -- verify that a test header field got added
     --   if not mt.eom_check(conn, MT_HDRINSERT, "Test-Header") then
     --           mt.echo("no header added ")
     --        error "no header added"
     --   end

       -- wrap it up!
       mt.echo("Test succeded")
       mt.disconnect(conn)
