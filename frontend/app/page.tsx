'use client'

import { useEffect, useState } from 'react'
import OpportunitiesList from '@/components/OpportunitiesList'
import MarketDataView from '@/components/MarketDataView'
import ConnectionStatus from '@/components/ConnectionStatus'

interface ArbitrageOpportunity {
  event_id: string
  buy_market: number
  sell_market: number
  buy_price: number
  sell_price: number
  profit_percentage: number
  max_size: number
}

interface MarketData {
  market_id: string
  market: number
  event_name: string
  best_bid: number
  best_ask: number
  bid_size: number
  ask_size: number
}

export default function Home() {
  const [opportunities, setOpportunities] = useState<ArbitrageOpportunity[]>([])
  const [marketData, setMarketData] = useState<Map<string, MarketData>>(new Map())
  const [connected, setConnected] = useState(false)

  useEffect(() => {
    const ws = new WebSocket('ws://localhost:8080')
    
    ws.onopen = () => {
      console.log('Connected to WebSocket server')
      setConnected(true)
    }
    
    ws.onmessage = (event) => {
      try {
        const message = JSON.parse(event.data)
        
        if (message.type === 'opportunity') {
          setOpportunities(prev => {
            const newOpps = [message.data, ...prev]
            return newOpps.slice(0, 50)
          })
        } else if (message.type === 'market_data') {
          setMarketData(prev => {
            const newMap = new Map(prev)
            newMap.set(message.data.market_id, message.data)
            return newMap
          })
        }
      } catch (e) {
        console.error('Error parsing WebSocket message:', e)
      }
    }
    
    ws.onerror = (error) => {
      console.error('WebSocket error:', error)
      setConnected(false)
    }
    
    ws.onclose = () => {
      console.log('WebSocket connection closed')
      setConnected(false)
    }
    
    return () => {
      ws.close()
    }
  }, [])

  return (
    <main className="container">
      <div className="header">
        <h1>Cross-Market Arbitrage Platform</h1>
        <ConnectionStatus connected={connected} />
      </div>
      
      <div className="grid">
        <div>
          <OpportunitiesList opportunities={opportunities} />
        </div>
        <div>
          <MarketDataView marketData={Array.from(marketData.values())} />
        </div>
      </div>
    </main>
  )
}

